#include "plugin/components/DelayedWeapon.hpp"

#include "NetworkShared.hpp"
#include "Weapon.hpp"
#include "WeaponHelpers.hpp"
#include "ecs/EmitEvent.hpp"
#include "ecs/EventManager.hpp"
#include "ecs/InitComponent.hpp"
#include "ecs/Registry.hpp"
#include "ecs/Scenes.hpp"
#include "ecs/zipper/Zipper.hpp"
#include "ecs/zipper/ZipperIndex.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/components/AnimatedSprite.hpp"
#include "plugin/components/Direction.hpp"
#include "plugin/components/Facing.hpp"
#include "plugin/components/Position.hpp"
#include "plugin/components/Team.hpp"
#include "plugin/events/EntityManagementEvent.hpp"

void Weapon::init_delayed_weapon(Ecs::Entity const& entity,
                                 JsonObject const& obj)
{
  auto fields = parse_base_weapon_fields<DelayedWeapon>(entity, obj);
  if (!fields.valid) {
    return;
  }

  auto const& delay_time = get_value<DelayedWeapon, double>(
      this->_registry.get(), obj, entity, "delay_time");
  if (!delay_time) {
    std::cerr << "Error loading DelayedWeapon component: unexpected value type "
                 "(delay_time: double)\n";
    return;
  }

  std::string attack_anim_name =
      fields.attack_animation ? fields.attack_animation.value() : "";
  double offset_x_value = fields.offset_x ? fields.offset_x.value() : 0.0;
  double offset_y_value = fields.offset_y ? fields.offset_y.value() : 0.0;

  init_component<DelayedWeapon>(this->_registry.get(),
                                this->_event_manager.get(),
                                entity,
                                fields.bullet_type.value(),
                                fields.magazine_size.value(),
                                fields.magazine_nb.value(),
                                fields.reload_time.value(),
                                fields.cooldown.value(),
                                delay_time.value(),
                                offset_x_value,
                                offset_y_value,
                                attack_anim_name);
}

void Weapon::delayed_weapon_system(
    std::chrono::high_resolution_clock::time_point now)
{
  // Handle reloading using helper function
  handle_reload_system<DelayedWeapon>(now);

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

      Position bullet_pos = pos;
      bullet_pos.pos.x += weapon.offset_x;
      bullet_pos.pos.y += weapon.offset_y;
      LoadEntityTemplate::Additional additional = {
          {this->_registry.get().get_component_key<Position>(),
           bullet_pos.to_bytes()},
          {this->_registry.get().get_component_key<Direction>(),
           direction.to_bytes()},
          {this->_registry.get().get_component_key<Team>(), team.to_bytes()}};

      if (this->_registry.get().has_component<Scene>(entity)) {
        additional.emplace_back(
            this->_registry.get().get_component_key<Scene>(),
            this->_registry.get()
                .get_components<Scene>()[entity]
                .value()
                .to_bytes());
      }

      this->_event_manager.get().emit<LoadEntityTemplate>(weapon.bullet_type,
                                                          additional);

      weapon.has_pending_shot = false;

      // Emit ComponentBuilder for pending shot state change
      emit_weapon_component_update(entity, weapon);
    }
  }
}

void Weapon::fire_delayed(Registry& r, const FireBullet& e)
{
  auto now = r.clock().now();
  // Handle DelayedWeapon
  if (this->_registry.get().has_component<DelayedWeapon, Position>(e.entity)) {
    auto& weapon =
        *this->_registry.get().get_components<DelayedWeapon>()[e.entity];

    // Try to play attack animation
    try_play_attack_animation(e.entity, weapon.attack_animation);

    // Check if weapon can fire (cooldown, ammo, etc.)
    if (!weapon.update_basic_weapon(now)) {
      return;
    }

    // Schedule the shot instead of firing immediately
    weapon.has_pending_shot = true;
    weapon.pending_shot_time = now;

    // Emit ComponentBuilder for pending shot state change and ammo update
    emit_weapon_component_update(e.entity, weapon);
  }
}
