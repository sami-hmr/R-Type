#include <iostream>

#include "Mob.hpp"

#include "ecs/zipper/ZipperIndex.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/components/Enemy.hpp"
#include "plugin/components/Position.hpp"

Mob::Mob(Registry& r, EntityLoader& l)
    : APlugin(r,
              l,
              {},
              {COMP_INIT(Enemy, Enemy, init_enemy),
               COMP_INIT(Spawner, Spawner, init_spawner)})
{
  _registry.get().register_component<Enemy>("mob:Enemy");
  _registry.get().register_component<Spawner>("mob:Spawner");

  _registry.get().add_system([this](Registry& r) { spawner_system(r); });
}

void Mob::init_enemy(Registry::Entity const& entity, JsonObject const& obj)
{
  auto const& type =
      get_value<Enemy, Enemy_type>(this->_registry.get(), obj, entity, "type");

  if (!type) {
    std::cerr << "Error loading Enemy component: unexpected value type or "
                 "missing value in JsonObject\n";
    return;
  }
  this->_registry.get().emplace_component<Enemy>(entity, type.value());
}

void Mob::init_spawner(Registry::Entity const& entity, JsonObject const& obj)
{
  auto const& spawn_pos = get_value<Spawner, Vector2D>(
      this->_registry.get(), obj, entity, "spawn_pos");
  auto const& entity_template = get_value<Spawner, std::string>(
      this->_registry.get(), obj, entity, "entity_template");
  auto const& spawn_interval = get_value<Spawner, double>(
      this->_registry.get(), obj, entity, "spawn_interval");
  auto const& spawn_delta = get_value<Spawner, double>(
      this->_registry.get(), obj, entity, "spawn_delta");
  auto const& max_spawns =
      get_value<Spawner, int>(this->_registry.get(), obj, entity, "max_spawns");
  auto const& current_spawns = get_value<Spawner, int>(
      this->_registry.get(), obj, entity, "current_spawns");
  auto const& active =
      get_value<Spawner, bool>(this->_registry.get(), obj, entity, "active");

  if (!spawn_pos || !entity_template || !spawn_interval || !spawn_delta
      || !max_spawns || !current_spawns || !active)
  {
    std::cerr << "Error loading Spawner component: unexpected value type or "
                 "missing value in JsonObject\n";
    return;
  }
  this->_registry.get().emplace_component<Spawner>(entity,
    spawn_pos.value(),
    entity_template.value(),
    spawn_interval.value(),
    spawn_delta.value(),
    max_spawns.value(),
    current_spawns.value(),
    active.value());
}

void Mob::spawner_system(Registry& r)
{
  return;
}

extern "C"
{
void* entry_point(Registry& r, EntityLoader& e)
{
  return new Mob(r, e);
}
}
