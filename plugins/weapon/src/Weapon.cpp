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

Weapon::Weapon(Registry& r, EntityLoader& l)
    : APlugin("weapon",
              r,
              l,
              {"moving"},
              {COMP_INIT(BasicWeapon, BasicWeapon, init_basic_weapon)})
    , entity_loader(l)
{
  REGISTER_COMPONENT(BasicWeapon)
  SUBSCRIBE_EVENT(KeyPressedEvent,
                  { this->on_fire(this->_registry.get(), event); })
  _registry.get().add_system([this](Registry& r)
                             { this->basic_weapon_system(r.clock().now()); });
}

void Weapon::on_fire(Registry& r, const KeyPressedEvent& e)
{
  if (!e.key_pressed.contains(Key::SPACE)) {
    return;
  }

  auto now = r.clock().now();
  for (auto&& [weapon, pos, scene] : Zipper<BasicWeapon, Position, Scene>(r)) {
    if (!weapon.update_basic_weapon(now)) {
      continue;
    }
    this->_registry.get().emit<EventBuilder>(
        this->_registry.get().get_event_key<LoadEntityTemplate>(),
        LoadEntityTemplate(
            weapon.bullet_type,
            LoadEntityTemplate::Additional {
                {this->_registry.get().get_component_key<Position>(),
                 pos.to_bytes()},
                {this->_registry.get().get_component_key<Scene>(),
                 scene.to_bytes()}})
            .to_bytes());
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

extern "C"
{
void* entry_point(Registry& r, EntityLoader& l)
{
  return new Weapon(r, l);
}
}
