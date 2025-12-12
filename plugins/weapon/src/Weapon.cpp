#include "Weapon.hpp"

#include "Json/JsonParser.hpp"
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
    : APlugin(r, l, {"moving"}, {COMP_INIT(BasicWeapon, BasicWeapon, init_basic_weapon)}), entity_loader(l)
{
    _registry.get().register_component<BasicWeapon>("weapon:BasicWeapon");
    _registry.get().on<KeyPressedEvent>("KeyPressedEvent", [this] (const KeyPressedEvent& e) {
        this->on_fire(this->_registry.get(), e);
    });
}


void Weapon::on_fire(Registry &r, const KeyPressedEvent &e)
{
  if (!e.key_pressed.contains(Key::SPACE)) {
    return;
  }

  SparseArray<Scene> scenes = r.get_components<Scene>();
  SparseArray<BasicWeapon> weapons = r.get_components<BasicWeapon>();

  SparseArray<Controllable> controllables = r.get_components<Controllable>();
  Registry::Entity player = 0;
  
  for (auto &&[i, _]: ZipperIndex(controllables)) {
    player = i;
  }

  for (auto &&[scene, weapon] : Zipper(scenes, weapons)) {
    if (scene.state != SceneState::MAIN) {
        continue;
    }
    std::optional<Registry::Entity> bullet = this->entity_loader.load_entity(JsonObject({{"template", JsonValue(weapon.bullet_type)}}));
    if (!bullet) {
        continue;
    }
    SparseArray<Position> &positions = r.get_components<Position>();
    positions.at(bullet.value())->pos = positions.at(player)->pos;
    r.add_component<Scene>(bullet.value(), Scene("game", SceneState::ACTIVE));
  }
}


extern "C"
{
void* entry_point(Registry& r, EntityLoader& l)
{
  return new Weapon(r, l);
}
}