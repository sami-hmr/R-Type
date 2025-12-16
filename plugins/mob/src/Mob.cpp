#include <iostream>

#include "Mob.hpp"

#include "NetworkShared.hpp"
#include "ecs/zipper/ZipperIndex.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/components/Enemy.hpp"
#include "plugin/components/Position.hpp"

Mob::Mob(Registry& r, EntityLoader& l)
    : APlugin("mob",
              r,
              l,
              {},
              {COMP_INIT(Enemy, Enemy, init_enemy),
               COMP_INIT(Spawner, Spawner, init_spawner)}), entity_loader(l)
{
  _registry.get().register_component<Enemy>("mob:Enemy");
  _registry.get().register_component<Spawner>("mob:Spawner");

  _registry.get().add_system([this](Registry& r) { spawner_system(r); });
}

void Mob::init_enemy(Registry::Entity const& entity, JsonObject const& obj)
{
  auto const& type =
      get_value<Enemy, int>(this->_registry.get(), obj, entity, "type");

  if (!type) {
    std::cerr << "Error loading Enemy component: unexpected value type or "
                 "missing value in JsonObject\n";
    return;
  }
  this->_registry.get().emplace_component<Enemy>(entity, Enemy_type(type.value()));
}

void Mob::init_spawner(Registry::Entity const& entity, JsonObject const& obj)
{
  auto const& spawn_pos = get_value<Spawner, Vector2D>(
      this->_registry.get(), obj, entity, "spawn_pos");
  auto const& entity_template = get_value<Spawner, std::string>(
      this->_registry.get(), obj, entity, "entity_template");
  auto const& spawn_delta = get_value<Spawner, double>(
      this->_registry.get(), obj, entity, "spawn_delta");
  auto const& max_spawns =
      get_value<Spawner, int>(this->_registry.get(), obj, entity, "max_spawns");

  if (!spawn_pos || !entity_template || !spawn_delta
      || !max_spawns)
  {
    std::cerr << "Error loading Spawner component: unexpected value type or "
                 "missing value in JsonObject\n";
    return;
  }
  this->_registry.get().emplace_component<Spawner>(entity,
    spawn_pos.value(),
    entity_template.value(),
    spawn_delta.value(),
    max_spawns.value());
}

void Mob::spawner_system(Registry& r)
{
  for (auto&& [i, spawner] : ZipperIndex<Spawner>(r)) {
    if (r.is_entity_dying(i)) {
      continue;
    }
    spawner.spawn_interval += r.clock().delta_seconds();
    if (spawner.spawn_interval < spawner.spawn_delta) {
      continue;
    }
    if (spawner.active && spawner.current_spawns < spawner.max_spawns) {
      spawner.spawn_interval = 0;
      spawner.current_spawns += 1;
      spawner.active = spawner.current_spawns < spawner.max_spawns;
      std::optional<Registry::Entity> enemy = this->entity_loader.load_entity(
        JsonObject({{"template", JsonValue(spawner.entity_template)}}));
      if (!enemy) {
        return;
      }
      SparseArray<Position>& positions = r.get_components<Position>();
      positions.at(enemy.value())->pos = spawner.spawn_pos;

      r.add_component<Scene>(enemy.value(), Scene("game", SceneState::ACTIVE));
      r.emit<ComponentBuilder>(
          i, r.get_component_key<Spawner>(), spawner.to_bytes());
    }
  }
}

extern "C"
{
void* entry_point(Registry& r, EntityLoader& e)
{
  return new Mob(r, e);
}
}
