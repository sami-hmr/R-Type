#include <vector>

#include "Weapon.hpp"

#include "NetworkShared.hpp"
#include "ecs/Registry.hpp"
#include "ecs/Scenes.hpp"
#include "ecs/zipper/Zipper.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/components/BasicWeapon.hpp"
#include "plugin/components/Position.hpp"
#include "plugin/events/EntityManagementEvent.hpp"
#include "plugin/events/IoEvents.hpp"
#include "plugin/events/WeaponEvent.hpp"
#include "ecs/EmitEvent.hpp"

Weapon::Weapon(Registry& r, EntityLoader& l)
    : APlugin("weapon",
              r,
              l,
              {"moving"},
              {COMP_INIT(BasicWeapon, BasicWeapon, init_basic_weapon)})
    , entity_loader(l)
{
  REGISTER_COMPONENT(BasicWeapon)
  SUBSCRIBE_EVENT(FireBullet, { this->on_fire(this->_registry.get(), event); })
  _registry.get().add_system([this](Registry& r)
                             { this->basic_weapon_system(r.clock().now()); });
}

void Weapon::on_fire(Registry& r, const FireBullet& e)
{
  auto now = r.clock().now();
  if (!this->_registry.get().has_component<BasicWeapon, Position, Scene>(
          e.entity))
  {
    return;
  }

  auto& weapon = *this->_registry.get().get_components<BasicWeapon>()[e.entity];
  auto const& pos =
      *this->_registry.get().get_components<Position>()[e.entity];
  auto const& scene =
      *this->_registry.get().get_components<Scene>()[e.entity];

  if (!weapon.update_basic_weapon(now)) {
    return;
  }
  emit_event(this->_registry.get(),
             "LoadEntityTemplate",
             LoadEntityTemplate(
                 weapon.bullet_type,
                 LoadEntityTemplate::Additional {
                     {this->_registry.get().get_component_key<Position>(),
                      pos.to_bytes()},
                     {this->_registry.get().get_component_key<Scene>(),
                      scene.to_bytes()}}));
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

extern "C"
{
void* entry_point(Registry& r, EntityLoader& l)
{
  return new Weapon(r, l);
}
}
