#include "plugin/components/ChargeWeapon.hpp"

#include "EntityExpose.hpp"
#include "NetworkShared.hpp"
#include "Weapon.hpp"
#include "WeaponHelpers.hpp"
#include "ecs/InitComponent.hpp"
#include "ecs/Registry.hpp"
#include "ecs/zipper/Zipper.hpp"
#include "ecs/zipper/ZipperIndex.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/components/AnimatedSprite.hpp"
#include "plugin/components/Direction.hpp"
#include "plugin/components/Facing.hpp"
#include "plugin/components/Position.hpp"
#include "plugin/components/ScaleModifier.hpp"
#include "plugin/components/Sprite.hpp"
#include "plugin/components/Team.hpp"
#include "plugin/events/AnimationEvents.hpp"
#include "plugin/events/EntityManagementEvent.hpp"
#include "plugin/events/WeaponEvent.hpp"

void Weapon::init_charge_weapon(Ecs::Entity const& entity,
                                JsonObject const& obj)
{
  auto fields = parse_base_weapon_fields<ChargeWeapon>(entity, obj);
  if (!fields.valid) {
    return;
  }

  auto const& charge_time = get_value<ChargeWeapon, double>(
      this->_registry.get(), obj, entity, "charge_time");
  if (!charge_time) {
    std::cerr << "Error loading ChargeWeapon component: unexpected value type "
                 "(charge_time: double)\n";
    return;
  }

  auto const& max_scale = get_value<ChargeWeapon, double>(
      this->_registry.get(), obj, entity, "max_scale");
  if (!max_scale) {
    std::cerr << "Error loading ChargeWeapon component: unexpected value type "
                 "(max_scale: double)\n";
    return;
  }

  auto const& min_charge_threshold = get_value<ChargeWeapon, double>(
      this->_registry.get(), obj, entity, "min_charge_threshold");
  if (!min_charge_threshold) {
    std::cerr << "Error loading ChargeWeapon component: unexpected value type "
                 "(min_charge_threshold: double)\n";
    return;
  }

  auto const& scale_damage = get_value<ChargeWeapon, bool>(
      this->_registry.get(), obj, entity, "scale_damage");
  if (!scale_damage) {
    std::cerr << "Error loading ChargeWeapon component: unexpected value type "
                 "(scale_damage: bool)\n";
    return;
  }

  auto const& charge_indicator = get_value<ChargeWeapon, std::string>(
      this->_registry.get(), obj, entity, "charge_indicator");
  std::string charge_indicator_name =
      charge_indicator ? charge_indicator.value() : "";

  std::string attack_anim_name =
      fields.attack_animation ? fields.attack_animation.value() : "";
  double offset_x_value = fields.offset_x ? fields.offset_x.value() : 0.0;
  double offset_y_value = fields.offset_y ? fields.offset_y.value() : 0.0;

  init_component<ChargeWeapon>(this->_registry.get(),
                               this->_event_manager.get(),
                               entity,
                               fields.bullet_type.value(),
                               fields.magazine_size.value(),
                               fields.magazine_nb.value(),
                               fields.reload_time.value(),
                               fields.cooldown.value(),
                               charge_time.value(),
                               max_scale.value(),
                               min_charge_threshold.value(),
                               scale_damage.value(),
                               offset_x_value,
                               offset_y_value,
                               attack_anim_name,
                               charge_indicator_name);
}

void Weapon::on_charge_release(Registry& /*unused*/,
                               const ReleaseChargeWeapon& e)
{
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

    // Emit ComponentBuilder for charge state change
    emit_weapon_component_update(e.entity, weapon);
    return;
  }

  // Play attack animation
  try_play_attack_animation(e.entity, weapon.attack_animation);

  auto const& pos =
      this->_registry.get().get_components<Position>()[e.entity].value();

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

  Offset offset(weapon.offset_x, weapon.offset_y);
  LoadEntityTemplate::Additional additional = {
      {this->_registry.get().get_component_key<Position>(), pos.to_bytes()},
      {this->_registry.get().get_component_key<Offset>(), offset.to_bytes()},
      {this->_registry.get().get_component_key<Direction>(),
       direction.to_bytes()},
      {this->_registry.get().get_component_key<Team>(), team.to_bytes()}};

  if (this->_registry.get().has_component<Scene>(e.entity)) {
    additional.emplace_back(this->_registry.get().get_component_key<Scene>(),
                            this->_registry.get()
                                .get_components<Scene>()[e.entity]
                                .value()
                                .to_bytes());
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
  }

  // Emit ComponentBuilder for charge state change
  emit_weapon_component_update(e.entity, weapon);
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

  // Emit ComponentBuilder for charge state change
  emit_weapon_component_update(e.entity, weapon);

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
      additional.emplace_back(this->_registry.get().get_component_key<Scene>(),
                              this->_registry.get()
                                  .get_components<Scene>()[e.entity]
                                  .value()
                                  .to_bytes());
    }

    this->_event_manager.get().emit<LoadEntityTemplate>(weapon.charge_indicator,
                                                        additional);
  } else {
    LOGGER("ChargeWeapon", LogLevel::ERR, "No charge indicator configured")
  }
}

void Weapon::charge_weapon_system(
    std::chrono::high_resolution_clock::time_point now)
{
  // Handle reloading using helper function
  handle_reload_system<ChargeWeapon>(now);

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

          // Emit ComponentBuilder when indicator entity is found
          emit_weapon_component_update(entity, weapon);
          break;
        }
      }
    }

    double old_charge_level = weapon.current_charge_level;
    double elapsed_time =
        std::chrono::duration<double>(now - weapon.charge_start_time).count();
    weapon.current_charge_level =
        std::min(1.0, elapsed_time / weapon.charge_time);

    // Emit ComponentBuilder for charge level change (every frame while
    // charging)
    if (weapon.current_charge_level != old_charge_level) {
      emit_weapon_component_update(entity, weapon);
    }

    if (weapon.charge_indicator_entity.has_value()) {
      double scale_factor =
          0.1 + (weapon.current_charge_level * (weapon.max_scale - 0.1));

      if (this->_registry.get().has_component<Sprite>(
              weapon.charge_indicator_entity.value()))
      {
        auto& sprite = *this->_registry.get().get_components<Sprite>()
                            [weapon.charge_indicator_entity.value()];
        sprite.scale = weapon.charge_indicator_base_scale * scale_factor;

        // Emit ComponentBuilder for sprite scale change
        this->_event_manager.get().emit<ComponentBuilder>(
            weapon.charge_indicator_entity.value(),
            this->_registry.get().get_component_key<Sprite>(),
            sprite.to_bytes());
      }

      Vector2D offset;
      if (this->_registry.get().has_component<AnimatedSprite>(
              weapon.charge_indicator_entity.value()))
      {
        auto& animated_sprite =
            *this->_registry.get().get_components<AnimatedSprite>()
                 [weapon.charge_indicator_entity.value()];

        animated_sprite.update_size(weapon.charge_indicator_base_scale
                                    * scale_factor);
        offset.x += (weapon.charge_indicator_base_scale.x * scale_factor) / 2;
        if (this->_registry.get().has_component<AnimatedSprite>(entity)) {
          auto const& ship_annim =
              this->_registry.get().get_components<AnimatedSprite>()[entity];
          offset.x += ship_annim->animations.at(ship_annim->current_animation)
                          .sprite_size.x
              / 2;
        }
        this->_event_manager.get().emit<ComponentBuilder>(
            weapon.charge_indicator_entity.value(),
            this->_registry.get().get_component_key<AnimatedSprite>(),
            animated_sprite.to_bytes());
      }

      if (this->_registry.get().has_component<Position>(
              weapon.charge_indicator_entity.value()))
      {
        auto& indicator_pos = *this->_registry.get().get_components<Position>()
                                   [weapon.charge_indicator_entity.value()];
        indicator_pos.pos = pos.pos + offset;

        // Emit ComponentBuilder for position update
        this->_event_manager.get().emit<ComponentBuilder>(
            weapon.charge_indicator_entity.value(),
            this->_registry.get().get_component_key<Position>(),
            indicator_pos.to_bytes());
      }
    }
  }
}
