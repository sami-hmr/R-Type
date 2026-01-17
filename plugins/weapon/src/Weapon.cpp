#include <iostream>
#include <optional>
#include <vector>

#include "Weapon.hpp"

#include "EntityExpose.hpp"
#include "NetworkShared.hpp"
#include "ecs/EmitEvent.hpp"
#include "ecs/EventManager.hpp"
#include "ecs/Registry.hpp"
#include "ecs/Scenes.hpp"
#include "ecs/zipper/Zipper.hpp"
#include "ecs/zipper/ZipperIndex.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/components/AnimatedSprite.hpp"
#include "plugin/components/BasicWeapon.hpp"
#include "plugin/components/ChargeWeapon.hpp"
#include "plugin/components/Damage.hpp"
#include "plugin/components/DelayedWeapon.hpp"
#include "plugin/components/Direction.hpp"
#include "plugin/components/Facing.hpp"
#include "plugin/components/Position.hpp"
#include "plugin/components/ScaleModifier.hpp"
#include "plugin/components/Sprite.hpp"
#include "plugin/components/Team.hpp"
#include "plugin/events/AnimationEvents.hpp"
#include "plugin/events/EntityManagementEvent.hpp"
#include "plugin/events/LogMacros.hpp"
#include "plugin/events/WeaponEvent.hpp"

Weapon::Weapon(Registry& r, EventManager& em, EntityLoader& l)
    : APlugin("weapon",
              r,
              em,
              l,
              {"moving", "life"},
              {COMP_INIT(BasicWeapon, BasicWeapon, init_basic_weapon),
               COMP_INIT(ChargeWeapon, ChargeWeapon, init_charge_weapon),
               COMP_INIT(DelayedWeapon, DelayedWeapon, init_delayed_weapon)})
    , entity_loader(l)
{
  REGISTER_COMPONENT(BasicWeapon)
  REGISTER_COMPONENT(ChargeWeapon)
  REGISTER_COMPONENT(DelayedWeapon)
  REGISTER_COMPONENT(ScaleModifier)
  SUBSCRIBE_EVENT(FireBullet, {
    if (this->_registry.get().has_component<ChargeWeapon>(event.entity)) {
      emit_event<StartChargeWeapon>(
          this->_event_manager.get(), "StartChargeWeapon", event.entity);
    } else {
      this->on_fire(this->_registry.get(), event);
    }
  })
  SUBSCRIBE_EVENT(StartChargeWeapon,
                  { this->on_charge_start(this->_registry.get(), event); })
  SUBSCRIBE_EVENT(ReleaseChargeWeapon,
                  { this->on_charge_release(this->_registry.get(), event); })
  _registry.get().add_system([this](Registry& r)
                             { this->basic_weapon_system(r.clock().now()); });
  _registry.get().add_system([this](Registry& r)
                             { this->charge_weapon_system(r.clock().now()); });
  _registry.get().add_system([this](Registry& r)
                             { this->delayed_weapon_system(r.clock().now()); });
  _registry.get().add_system([this](Registry& r)
                             { this->apply_scale_modifiers(); });
}

void Weapon::on_fire(Registry& r, const FireBullet& e)
{
  auto now = r.clock().now();

  if (this->_registry.get().has_component<BasicWeapon, Position>(e.entity)) {
    auto& weapon =
        *this->_registry.get().get_components<BasicWeapon>()[e.entity];

    // Try to play attack animation if entity has AnimatedSprite and weapon has
    // attack_animation
    if (!weapon.attack_animation.empty()
        && this->_registry.get().has_component<AnimatedSprite>(e.entity))
    {
      auto& sprite =
          *this->_registry.get().get_components<AnimatedSprite>()[e.entity];
      // Check if the specified animation exists
      if (sprite.animations.find(weapon.attack_animation)
          != sprite.animations.end())
      {
        auto& attack_anim = sprite.animations[weapon.attack_animation];
        this->_event_manager.get().emit<PlayAnimationEvent>(
            weapon.attack_animation,
            e.entity,
            attack_anim.framerate,
            attack_anim.loop,
            attack_anim.rollback);
      }
    }

    auto const& pos =
        *this->_registry.get().get_components<Position>()[e.entity];

    auto const& vel_direction =
        (this->_registry.get().has_component<Direction>(e.entity))
        ? this->_registry.get().get_components<Direction>()[e.entity]->direction
        : Vector2D(0, 0);

    auto const& fire_direction =
        (this->_registry.get().has_component<Facing>(e.entity))
        ? this->_registry.get().get_components<Facing>()[e.entity]->direction
        : vel_direction;

    auto direction = Direction(fire_direction);

    auto const& team = (this->_registry.get().has_component<Team>(e.entity))
        ? *this->_registry.get().get_components<Team>()[e.entity]
        : std::string("");

    if (!weapon.update_basic_weapon(now)) {
      return;
    }

    LoadEntityTemplate::Additional additional = {
        {this->_registry.get().get_component_key<Position>(), pos.to_bytes()},
        {this->_registry.get().get_component_key<Direction>(),
         direction.to_bytes()},
        {this->_registry.get().get_component_key<Team>(), team.to_bytes()}};

    if (this->_registry.get().has_component<Scene>(e.entity)) {
      additional.push_back({this->_registry.get().get_component_key<Scene>(),
                            this->_registry.get()
                                .get_components<Scene>()[e.entity]
                                .value()
                                .to_bytes()});
    }
    this->_event_manager.get().emit<LoadEntityTemplate>(weapon.bullet_type,
                                                        additional);
    return;
  }

  // Handle DelayedWeapon
  if (this->_registry.get().has_component<DelayedWeapon, Position>(e.entity)) {
    auto& weapon =
        *this->_registry.get().get_components<DelayedWeapon>()[e.entity];

    // Try to play attack animation if entity has AnimatedSprite and weapon has
    // attack_animation
    if (!weapon.attack_animation.empty()
        && this->_registry.get().has_component<AnimatedSprite>(e.entity))
    {
      auto& sprite =
          *this->_registry.get().get_components<AnimatedSprite>()[e.entity];
      // Check if the specified animation exists
      if (sprite.animations.find(weapon.attack_animation)
          != sprite.animations.end())
      {
        auto& attack_anim = sprite.animations[weapon.attack_animation];
        this->_event_manager.get().emit<PlayAnimationEvent>(
            weapon.attack_animation,
            e.entity,
            attack_anim.framerate,
            attack_anim.loop,
            attack_anim.rollback);
      }
    }

    // Check if weapon can fire (cooldown, ammo, etc.)
    if (!weapon.update_basic_weapon(now)) {
      return;
    }

    // Schedule the shot instead of firing immediately
    weapon.has_pending_shot = true;
    weapon.pending_shot_time = now;
  }
}

void Weapon::on_charge_start(Registry& r, const StartChargeWeapon& e)
{
  auto now = r.clock().now();

  if (!this->_registry.get().has_component<ChargeWeapon, Position>(e.entity)) {
    return;
  }

  auto& weapon =
      *this->_registry.get().get_components<ChargeWeapon>()[e.entity];

  if (weapon.charge_indicator_entity.has_value()) {
    return;
  }

  if (!weapon.update_basic_weapon(now)) {
    return;
  }

  weapon.is_charging = true;
  weapon.charge_start_time = now;
  weapon.current_charge_level = 0.0;
  ;
  if (!weapon.charge_indicator.empty()
      && !weapon.charge_indicator_entity.has_value())
  {
    auto const& pos =
        *this->_registry.get().get_components<Position>()[e.entity];

    LoadEntityTemplate::Additional additional = {
        {this->_registry.get().get_component_key<Position>(), pos.to_bytes()},
        {this->_registry.get().get_component_key<IdStorage>(),
         IdStorage(e.entity, "charge_weapon_indicator").to_bytes()}};

    if (this->_registry.get().has_component<Scene>(e.entity)) {
      additional.push_back({this->_registry.get().get_component_key<Scene>(),
                            this->_registry.get()
                                .get_components<Scene>()[e.entity]
                                .value()
                                .to_bytes()});
    }

    this->_event_manager.get().emit<LoadEntityTemplate>(weapon.charge_indicator,
                                                        additional);
  } else {
    LOGGER("ChargeWeapon", LogLevel::ERR, "No charge indicator configured")
  }
}

void Weapon::on_charge_release(Registry& r, const ReleaseChargeWeapon& e)
{
  auto now = r.clock().now();

  if (!this->_registry.get().has_component<ChargeWeapon, Position>(e.entity)) {
    return;
  }

  auto& weapon =
      *this->_registry.get().get_components<ChargeWeapon>()[e.entity];

  if (!weapon.is_charging) {
    return;
  }

  if (weapon.current_charge_level < weapon.min_charge_threshold) {
    weapon.is_charging = false;
    weapon.current_charge_level = 0.0;

    if (weapon.charge_indicator_entity.has_value()) {
      this->_event_manager.get().emit<DeleteEntity>(
          weapon.charge_indicator_entity.value());
      weapon.charge_indicator_entity = std::nullopt;
    }

    return;
  }

  if (!weapon.attack_animation.empty()
      && this->_registry.get().has_component<AnimatedSprite>(e.entity))
  {
    auto& sprite =
        *this->_registry.get().get_components<AnimatedSprite>()[e.entity];
    if (sprite.animations.find(weapon.attack_animation)
        != sprite.animations.end())
    {
      auto& attack_anim = sprite.animations[weapon.attack_animation];
      this->_event_manager.get().emit<PlayAnimationEvent>(
          weapon.attack_animation,
          e.entity,
          attack_anim.framerate,
          attack_anim.loop,
          attack_anim.rollback);
    }
  }

  auto const& pos = *this->_registry.get().get_components<Position>()[e.entity];

  auto const& vel_direction =
      (this->_registry.get().has_component<Direction>(e.entity))
      ? this->_registry.get().get_components<Direction>()[e.entity]->direction
      : Vector2D(0, 0);

  auto const& fire_direction =
      (this->_registry.get().has_component<Facing>(e.entity))
      ? this->_registry.get().get_components<Facing>()[e.entity]->direction
      : vel_direction;

  auto direction = Direction(fire_direction);

  auto const& team = (this->_registry.get().has_component<Team>(e.entity))
      ? *this->_registry.get().get_components<Team>()[e.entity]
      : std::string("");

  double scale_multiplier =
      1.0 + (weapon.current_charge_level * (weapon.max_scale - 1.0));

  LoadEntityTemplate::Additional additional = {
      {this->_registry.get().get_component_key<Position>(), pos.to_bytes()},
      {this->_registry.get().get_component_key<Direction>(),
       direction.to_bytes()},
      {this->_registry.get().get_component_key<Team>(), team.to_bytes()}};

  if (this->_registry.get().has_component<Scene>(e.entity)) {
    additional.push_back({this->_registry.get().get_component_key<Scene>(),
                          this->_registry.get()
                              .get_components<Scene>()[e.entity]
                              .value()
                              .to_bytes()});
  }

  additional.emplace_back(
      this->_registry.get().get_component_key<ScaleModifier>(),
      ScaleModifier(scale_multiplier, weapon.scale_damage).to_bytes());

  this->_event_manager.get().emit<LoadEntityTemplate>(weapon.bullet_type,
                                                      additional);

  weapon.is_charging = false;
  weapon.current_charge_level = 0.0;

  if (weapon.charge_indicator_entity.has_value()) {
    this->_event_manager.get().emit<DeleteEntity>(
        weapon.charge_indicator_entity.value());
    weapon.charge_indicator_entity = std::nullopt;
  } else {
    // std::cout << "[ChargeWeapon] No charge indicator to destroy" <<
    // std::endl;
  }
}

void Weapon::basic_weapon_system(
    std::chrono::high_resolution_clock::time_point now)
{
  for (auto&& [weapon] : Zipper<BasicWeapon>(_registry.get())) {
    if (weapon.reloading && weapon.remaining_magazine > 0) {
      double elapsed_time =
          std::chrono::duration<double>(now - weapon.last_reload_time).count();
      if (elapsed_time >= weapon.reload_time) {
        weapon.reloading = false;
        weapon.remaining_ammo = weapon.magazine_size;
        weapon.remaining_magazine -= 1;
      }
    }
  }
}

void Weapon::charge_weapon_system(
    std::chrono::high_resolution_clock::time_point now)
{
  // Handle reloading
  for (auto&& [weapon] : Zipper<ChargeWeapon>(_registry.get())) {
    if (weapon.reloading && weapon.remaining_magazine > 0) {
      double elapsed_time =
          std::chrono::duration<double>(now - weapon.last_reload_time).count();
      if (elapsed_time >= weapon.reload_time) {
        weapon.reloading = false;
        weapon.remaining_ammo = weapon.magazine_size;
        weapon.remaining_magazine -= 1;
      }
    }
  }

  // Handle charging weapons
  for (auto&& [entity, weapon, pos] :
       ZipperIndex<ChargeWeapon, Position>(_registry.get()))
  {
    if (!weapon.is_charging) {
      continue;
    }

    // Find and store the charge indicator entity if we haven't already
    // Keep searching each frame until we find it (LoadEntityTemplate is async)
    if (!weapon.charge_indicator_entity.has_value()) {
      for (auto&& [indicator_entity, marker] :
           ZipperIndex<IdStorage>(this->_registry.get()))
      {
        if (marker.id_s == entity
            && marker.context == "charge_weapon_indicator")
        {
          weapon.charge_indicator_entity = indicator_entity;

          if (this->_registry.get().has_component<Sprite>(indicator_entity)) {
            auto& sprite = *this->_registry.get()
                                .get_components<Sprite>()[indicator_entity];
            weapon.charge_indicator_base_scale = sprite.scale;
          } else if (this->_registry.get().has_component<AnimatedSprite>(
                         indicator_entity))
          {
            auto& anim =
                *this->_registry.get()
                     .get_components<AnimatedSprite>()[indicator_entity];
            auto anim_it = anim.animations.find(anim.current_animation);
            if (anim_it != anim.animations.end()) {
              weapon.charge_indicator_base_scale = anim_it->second.sprite_size;
            }
          }
          break;
        }
      }
    }

    double elapsed_time =
        std::chrono::duration<double>(now - weapon.charge_start_time).count();
    weapon.current_charge_level =
        std::min(1.0, elapsed_time / weapon.charge_time);

    if (weapon.charge_indicator_entity.has_value()) {
      double scale_factor =
          0.1 + (weapon.current_charge_level * (weapon.max_scale - 0.1));

      if (this->_registry.get().has_component<Position>(
              weapon.charge_indicator_entity.value()))
      {
        auto& indicator_pos = *this->_registry.get().get_components<Position>()
                                   [weapon.charge_indicator_entity.value()];
        indicator_pos.pos = pos.pos;
      }

      if (this->_registry.get().has_component<Sprite>(
              weapon.charge_indicator_entity.value()))
      {
        auto& sprite = *this->_registry.get().get_components<Sprite>()
                            [weapon.charge_indicator_entity.value()];
        sprite.scale = weapon.charge_indicator_base_scale * scale_factor;
      }

      if (this->_registry.get().has_component<AnimatedSprite>(
              weapon.charge_indicator_entity.value()))
      {
        auto& animated_sprite =
            *this->_registry.get().get_components<AnimatedSprite>()
                 [weapon.charge_indicator_entity.value()];

        animated_sprite.update_size(weapon.charge_indicator_base_scale
                                    * scale_factor);
        this->_event_manager.get().emit<ComponentBuilder>(
            weapon.charge_indicator_entity.value(),
            this->_registry.get().get_component_key<AnimatedSprite>(),
            animated_sprite.to_bytes());
      }
    }
  }
}

void Weapon::delayed_weapon_system(
    std::chrono::high_resolution_clock::time_point now)
{
  for (auto&& [weapon] : Zipper<DelayedWeapon>(_registry.get())) {
    if (weapon.reloading && weapon.remaining_magazine > 0) {
      double elapsed_time =
          std::chrono::duration<double>(now - weapon.last_reload_time).count();
      if (elapsed_time >= weapon.reload_time) {
        weapon.reloading = false;
        weapon.remaining_ammo = weapon.magazine_size;
        weapon.remaining_magazine -= 1;
      }
    }
  }

  for (auto&& [entity, weapon, pos] :
       ZipperIndex<DelayedWeapon, Position>(_registry.get()))
  {
    if (!weapon.has_pending_shot) {
      continue;
    }

    double elapsed_time =
        std::chrono::duration<double>(now - weapon.pending_shot_time).count();

    if (elapsed_time >= weapon.delay_time) {
      auto const& vel_direction =
          (this->_registry.get().has_component<Direction>(entity))
          ? this->_registry.get().get_components<Direction>()[entity]->direction
          : Vector2D(0, 0);

      auto const& fire_direction =
          (this->_registry.get().has_component<Facing>(entity))
          ? this->_registry.get().get_components<Facing>()[entity]->direction
          : vel_direction;

      auto direction = Direction(fire_direction);

      auto const& team = (this->_registry.get().has_component<Team>(entity))
          ? *this->_registry.get().get_components<Team>()[entity]
          : std::string("");

      LoadEntityTemplate::Additional additional = {
          {this->_registry.get().get_component_key<Position>(), pos.to_bytes()},
          {this->_registry.get().get_component_key<Direction>(),
           direction.to_bytes()},
          {this->_registry.get().get_component_key<Team>(), team.to_bytes()}};

      if (this->_registry.get().has_component<Scene>(entity)) {
        additional.push_back({this->_registry.get().get_component_key<Scene>(),
                              this->_registry.get()
                                  .get_components<Scene>()[entity]
                                  .value()
                                  .to_bytes()});
      }

      this->_event_manager.get().emit<LoadEntityTemplate>(weapon.bullet_type,
                                                          additional);

      weapon.has_pending_shot = false;
    }
  }
}

void Weapon::apply_scale_modifiers()
{
  for (auto&& [entity, modifier, sprite] :
       ZipperIndex<ScaleModifier, Sprite>(this->_registry.get()))
  {
    if (!modifier.applied) {
      sprite.scale = sprite.scale * modifier.scale_multiplier;

      if (modifier.scale_damage
          && this->_registry.get().has_component<Damage>(entity))
      {
        auto& damage = *this->_registry.get().get_components<Damage>()[entity];
        damage.amount =
            static_cast<int>(damage.amount * modifier.scale_multiplier);
      }

      modifier.applied = true;
    }
  }

  for (auto&& [entity, modifier, animated_sprite] :
       ZipperIndex<ScaleModifier, AnimatedSprite>(this->_registry.get()))
  {
    if (!modifier.applied) {
      auto anim_it =
          animated_sprite.animations.find(animated_sprite.current_animation);
      if (anim_it != animated_sprite.animations.end()) {
        anim_it->second.sprite_size =
            anim_it->second.sprite_size * modifier.scale_multiplier;
      }

      if (modifier.scale_damage
          && this->_registry.get().has_component<Damage>(entity))
      {
        auto& damage = *this->_registry.get().get_components<Damage>()[entity];
        damage.amount =
            static_cast<int>(damage.amount * modifier.scale_multiplier);
      }

      modifier.applied = true;
    }
  }
}

extern "C"
{
PLUGIN_EXPORT void* entry_point(Registry& r, EventManager& em, EntityLoader& l)
{
  return new Weapon(r, em, l);
}
}
