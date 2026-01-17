#include <iostream>
#include <optional>

#include "Mob.hpp"

#include "Json/JsonParser.hpp"
#include "NetworkShared.hpp"
#include "ecs/EventManager.hpp"
#include "ecs/zipper/ZipperIndex.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/components/Direction.hpp"
#include "plugin/components/Health.hpp"
#include "plugin/components/MovementBehavior.hpp"
#include "plugin/components/Parasite.hpp"
#include "plugin/components/Position.hpp"
#include "plugin/components/Spawner.hpp"
#include "plugin/components/Speed.hpp"
#include "plugin/events/EntityManagementEvent.hpp"
#include "plugin/events/InteractionZoneEvent.hpp"

Mob::Mob(Registry& r, EventManager& em, EntityLoader& l)
    : APlugin("mob",
              r,
              em,
              l,
              {"moving", "collision", "life", "ai"},
              {COMP_INIT(Spawner, Spawner, init_spawner),
               COMP_INIT(Parasite, Parasite, init_parasite)})
    , entity_loader(l)
{
  REGISTER_COMPONENT(Spawner)
  REGISTER_COMPONENT(Parasite)

  _registry.get().add_system([this](Registry& r) { spawner_system(r); });
  _registry.get().add_system([this](Registry& r) { parasite_system(r); });

  SUBSCRIBE_EVENT(InteractionZoneEvent, { this->on_interaction_zone(event); })
}

void Mob::init_spawner(Ecs::Entity const& entity, JsonObject const& obj)
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

void Mob::init_parasite(Ecs::Entity const& entity, JsonObject const& obj)
{
  auto const& behaviour = get_value<Parasite, std::string>(
      this->_registry.get(), obj, entity, "behaviour");

  std::optional<std::size_t> id = std::nullopt;
  if (obj.contains("entity_id")) {
    id = get_value<Parasite, std::size_t>(
        this->_registry.get(), obj, entity, "entity_id");
  }

  if (!behaviour) {
    std::cerr << "Error loading Parasite component: unexpected value type or "
                 "missing value in JsonObject\n";
    return;
  }
  this->_registry.get().emplace_component<Parasite>(
      entity, id, behaviour.value());
}

void Mob::on_interaction_zone(const InteractionZoneEvent& event)
{
  const auto& positions = this->_registry.get().get_components<Position>();
  auto& parasite = this->_registry.get().get_components<Parasite>();

  if (!this->_registry.get().has_component<Parasite>(event.source)
      || parasite[event.source]->entity_id.has_value())
  {
    return;
  }

  std::optional<Ecs::Entity> closest_entity = std::nullopt;
  double closest_distance_sq = event.radius * event.radius;

  for (const Ecs::Entity& candidate : event.candidates) {
    if (!this->_registry.get().has_component<Health>(candidate)) {
      continue;
    }
    Vector2D distance =
        positions[candidate]->pos - positions[event.source]->pos;
    double distance_sq = distance.length();

    if (distance_sq < closest_distance_sq) {
      closest_distance_sq = distance_sq;
      closest_entity = candidate;
    }
  }
  if (closest_entity.has_value()) {
    parasite[event.source]->entity_id.emplace(closest_entity.value());

    this->_event_manager.get().emit<ComponentBuilder>(
        event.source,
        this->_registry.get().get_component_key<Parasite>(),
        parasite[event.source]->to_bytes());
  }
}

void Mob::parasite_system(Registry& r)
{
  for (auto&& [i, parasite, pos, speed, direction] :
       ZipperIndex<Parasite, Position, Speed, Direction>(r))
  {
    if (r.is_entity_dying(i)) {
      continue;
    }
    if (!parasite.entity_id.has_value()) {
      continue;
    }
    JsonObject params;
    params.emplace("target_id",
                   JsonValue(static_cast<int>(parasite.entity_id.value())));

    if (this->_registry.get().has_component<MovementBehavior>(i)) {
      auto& behavior =
          this->_registry.get().get_components<MovementBehavior>()[i].value();
      params.merge(behavior.params);
      behavior.movement_type = parasite.behavior;
      behavior.params = params;
    } else {
      this->_registry.get().add_component(
          i, MovementBehavior(parasite.behavior, params));
    }
  }
}

void Mob::spawner_system(Registry& r)
{
  std::vector<std::function<void()>> event_to_emit;

  for (auto&& [i, spawner, pos] : ZipperIndex<Spawner, Position>(r)) {
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
      event_to_emit.emplace_back(
          [this,
           i = i,
           spawner_bytes = spawner.to_bytes(),
           entity_template = spawner.entity_template,
           pos_bytes = pos.to_bytes()]()
          {
            this->_event_manager.get().emit<ComponentBuilder>(
                i,
                this->_registry.get().get_component_key<Spawner>(),
                spawner_bytes);
            LoadEntityTemplate::Additional additional = {
                {this->_registry.get().get_component_key<Position>(),
                 pos_bytes}};

            if (this->_registry.get().has_component<Scene>(i)) {
              additional.push_back(
                  {this->_registry.get().get_component_key<Scene>(),
                   this->_registry.get()
                       .get_components<Scene>()[i]
                       .value()
                       .to_bytes()});
            }
            this->_event_manager.get().emit<LoadEntityTemplate>(entity_template,
                                                                additional);
          });
    }
  }
  for (auto const& fn : event_to_emit) {
    fn();
  }
}

extern "C"
{
void* entry_point(Registry& r, EventManager& em, EntityLoader& e)
{
  return new Mob(r, em, e);
}
}
