#include "plugin/components/BasicWeapon.hpp"

#include "NetworkShared.hpp"
#include "Weapon.hpp"
#include "WeaponHelpers.hpp"
#include "ecs/InitComponent.hpp"
#include "ecs/Registry.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/components/AnimatedSprite.hpp"
#include "plugin/components/Direction.hpp"
#include "plugin/components/Facing.hpp"
#include "plugin/components/Position.hpp"
#include "plugin/components/Team.hpp"
#include "plugin/events/EntityManagementEvent.hpp"

void Weapon::init_basic_weapon(Ecs::Entity const& entity, JsonObject const& obj)
{
  auto fields = parse_base_weapon_fields<BasicWeapon>(entity, obj);
  if (!fields.valid) {
    return;
  }

  std::string attack_anim_name =
      fields.attack_animation ? fields.attack_animation.value() : "";
  double offset_x_value = fields.offset_x ? fields.offset_x.value() : 0.0;
  double offset_y_value = fields.offset_y ? fields.offset_y.value() : 0.0;

  init_component<BasicWeapon>(this->_registry.get(),
                              this->_event_manager.get(),
                              entity,
                              fields.bullet_type.value(),
                              fields.magazine_size.value(),
                              fields.magazine_nb.value(),
                              fields.reload_time.value(),
                              fields.cooldown.value(),
                              offset_x_value,
                              offset_y_value,
                              attack_anim_name);
}

void Weapon::fire_basic(Registry& r, const FireBullet& e)
{
  auto now = r.clock().now();

  if (this->_registry.get().has_component<BasicWeapon, Position>(e.entity)) {
    auto& weapon =
        *this->_registry.get().get_components<BasicWeapon>()[e.entity];

    // Try to play attack animation
    try_play_attack_animation(e.entity, weapon.attack_animation);

    auto& pos = *this->_registry.get().get_components<Position>()[e.entity];

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

    // Emit ComponentBuilder for weapon state change (ammo/cooldown updated)
    emit_weapon_component_update(e.entity, weapon);

    Position new_pos = pos;
    new_pos.pos.x += weapon.offset_x;
    new_pos.pos.y += weapon.offset_y;
    LoadEntityTemplate::Additional additional = {
        {this->_registry.get().get_component_key<Position>(),
         new_pos.to_bytes()},
        {this->_registry.get().get_component_key<Direction>(),
         direction.to_bytes()},
        {this->_registry.get().get_component_key<Team>(), team.to_bytes()},
        {this->_registry.get().get_component_key<Facing>(),
         Facing(direction.direction, /*plane=*/true).to_bytes()}};

    if (this->_registry.get().has_component<Scene>(e.entity)) {
      additional.emplace_back(this->_registry.get().get_component_key<Scene>(),
                              this->_registry.get()
                                  .get_components<Scene>()[e.entity]
                                  .value()
                                  .to_bytes());
    }
    this->_event_manager.get().emit<LoadEntityTemplate>(weapon.bullet_type,
                                                        additional);
    return;
  }
}
