#include "Weapon.hpp"

#include "Json/JsonParser.hpp"
#include "NetworkShared.hpp"
#include "ecs/Registry.hpp"
#include "ecs/Scenes.hpp"
#include "ecs/SparseArray.hpp"
#include "ecs/zipper/Zipper.hpp"
#include "ecs/zipper/ZipperIndex.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/components/BasicWeapon.hpp"
#include "plugin/components/Controllable.hpp"
#include "plugin/components/Position.hpp"
#include "plugin/components/Velocity.hpp"
#include "plugin/events/IoEvents.hpp"

Weapon::Weapon(Registry& r, EntityLoader& l)
    : APlugin(r,
              l,
              {"moving"},
              {COMP_INIT(BasicWeapon, BasicWeapon, init_basic_weapon)})
    , entity_loader(l)
{
  _registry.get().register_component<BasicWeapon>("weapon:BasicWeapon");
  _registry.get().on<KeyPressedEvent>(
      "KeyPressedEvent",
      [this](const KeyPressedEvent& e)
      { this->on_fire(this->_registry.get(), e); });
  _registry.get().add_system([this](Registry& /*unused*/)
                             { this->basic_weapon_system(); });
}

void Weapon::on_fire(Registry& r, const KeyPressedEvent& e)
{
  if (!e.key_pressed.contains(Key::SPACE)) {
    return;
  }

  for (auto&& [weapon, pos] : Zipper<BasicWeapon, Position>(r)) {
    weapon.update_basic_weapon();
    if (weapon.remaining_ammo <= 0 || weapon.reloading) {
      continue;
    }
    Vector2D spawn_pos = pos.pos;
    std::optional<Registry::Entity> bullet = this->entity_loader.load_entity(
        JsonObject({{"template", JsonValue(weapon.bullet_type)}}));
    if (!bullet) {
      continue;
    }
    SparseArray<Position>& positions = r.get_components<Position>();
    positions.at(bullet.value())->pos = spawn_pos;
    r.add_component<Scene>(bullet.value(), Scene("game", SceneState::ACTIVE));
  }
}

void Weapon::basic_weapon_system()
{
  for (auto&& [weapon] : Zipper<BasicWeapon>(_registry.get())) {
    if (weapon.reloading && weapon.remaining_magazine > 0) {
      auto now = std::chrono::high_resolution_clock::now();
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
