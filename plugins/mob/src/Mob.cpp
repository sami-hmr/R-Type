#include <iostream>
#include <optional>

#include "Mob.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/Hooks.hpp"
#include "NetworkShared.hpp"
#include "ecs/EventManager.hpp"
#include "plugin/EntityLoader.hpp"
#include "ecs/zipper/ZipperIndex.hpp"
#include "plugin/components/Spawner.hpp"
#include "plugin/components/Position.hpp"
#include "plugin/components/Parasite.hpp"
#include "plugin/components/Speed.hpp"
#include "plugin/events/EntityManagementEvent.hpp"

Mob::Mob(Registry& r, EventManager &em, EntityLoader& l)
    : APlugin(
          "mob", r, em, l, {"moving"}, {COMP_INIT(Spawner, Spawner, init_spawner)})
    , entity_loader(l)
{
  _registry.get().register_component<Spawner>("mob:Spawner");

  _registry.get().add_system([this](Registry& r) { spawner_system(r); });
}

void Mob::init_spawner(Registry::Entity const& entity, JsonObject const& obj)
{
  auto const& entity_template = get_value<Spawner, std::string>(
      this->_registry.get(), obj, entity, "entity_template");
  auto const& spawn_interval = get_value<Spawner, double>(
      this->_registry.get(), obj, entity, "spawn_interval");
  auto const& max_spawns =
      get_value<Spawner, int>(this->_registry.get(), obj, entity, "max_spawns");

  if (!entity_template || !spawn_interval || !max_spawns) {
    std::cerr << "Error loading Spawner component: unexpected value type or "
                 "missing value in JsonObject\n";
    return;
  }
  this->_registry.get().emplace_component<Spawner>(entity,
                                                   entity_template.value(),
                                                   spawn_interval.value(),
                                                   max_spawns.value());
}

void Mob::init_parasite(Registry::Entity const& entity, JsonObject const& obj)
{
  auto const& behaviour = get_value<Parasite, std::string>(
      this->_registry.get(), obj, entity, "behaviour");
  auto const& effect = get_value<Parasite, std::string>(
      this->_registry.get(), obj, entity, "effect");
  auto const& dflt_speed =
      get_value<Parasite, Vector2D>(this->_registry.get(), obj, entity, "default_speed");

  if (!behaviour || !effect || !dflt_speed) {
    std::cerr << "Error loading Parasite component: unexpected value type or "
                 "missing value in JsonObject\n";
    return;
  }
  this->_registry.get().emplace_component<Parasite>(entity,
                                                   behaviour.value(),
                                                   effect.value(),
                                                   dflt_speed.value());
}

void Mob::parasite_system(Registry& r)
{
  for (auto&& [i, parasite, pos, speed, scene] :
       ZipperIndex<Parasite, Position, Speed, Scene>(r))
  {
    if (r.is_entity_dying(i)) {
      continue;
    }
    if (parasite.player_linked == std::nullopt) {
      speed.speed = parasite.dflt_speed;
    }
  }
}

void Mob::spawner_system(Registry& r)
{
  for (auto&& [i, spawner, pos, scene] :
       ZipperIndex<Spawner, Position, Scene>(r))
  {
    if (r.is_entity_dying(i)) {
      continue;
    }
    if (!spawner.active) {
      continue;
    }
    spawner.spawn_delta += r.clock().delta_seconds();
    if (spawner.spawn_delta < spawner.spawn_interval) {
      continue;
    }
    if (spawner.active && spawner.current_spawns < spawner.max_spawns) {
      spawner.spawn_delta = 0;
      spawner.current_spawns += 1;
      spawner.active = spawner.current_spawns < spawner.max_spawns;
      if (spawner.current_spawns >= spawner.max_spawns) {
        spawner.active = false;
      }
      this->_event_manager.get().emit<ComponentBuilder>(
          i, r.get_component_key<Spawner>(), spawner.to_bytes());
      this->_event_manager.get().emit<LoadEntityTemplate>(
          spawner.entity_template,
          LoadEntityTemplate::Additional {
              {
                  this->_registry.get().get_component_key<Position>(),
                  pos.to_bytes(),
              },
              {this->_registry.get().get_component_key<Scene>(),
               scene.to_bytes()}});
    }
  }
}

extern "C"
{
void* entry_point(Registry& r, EventManager &em, EntityLoader& e)
{
  return new Mob(r, em, e);
}
}
