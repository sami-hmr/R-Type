#include "Target.hpp"

#include "Json/JsonParser.hpp"
#include "NetworkShared.hpp"
#include "ecs/EventManager.hpp"
#include "ecs/InitComponent.hpp"
#include "ecs/Registry.hpp"
#include "ecs/SparseArray.hpp"
#include "ecs/zipper/ZipperIndex.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/components/Direction.hpp"
#include "plugin/components/Facing.hpp"
#include "plugin/components/Follower.hpp"
#include "plugin/components/Health.hpp"
#include "plugin/components/Position.hpp"
#include "plugin/components/Speed.hpp"
#include "plugin/components/Team.hpp"
#include "plugin/events/InteractionZoneEvent.hpp"

Target::Target(Registry& r, EventManager& em, EntityLoader& l)
    : APlugin("target",
              r,
              em,
              l,
              {"moving", "life"},
              {COMP_INIT(Follower, Follower, init_follower)})
{
  REGISTER_COMPONENT(Follower)
  this->_registry.get().add_system([this](Registry& r)
                                   { this->target_system(r); });
  SUBSCRIBE_EVENT(InteractionZoneEvent, { this->on_interaction_zone(event); })
}

void Target::init_follower(Registry::Entity entity, JsonObject const& /*obj*/)
{
  init_component<Follower>(
      this->_registry.get(), this->_event_manager.get(), entity);
}

void Target::target_system(Registry& reg)
{
  auto const& positions = reg.get_components<Position>();
  auto& faces = reg.get_components<Facing>();

  for (auto&& [i, follower, position, direction, speed] :
       ZipperIndex<Follower, Position, Direction, Speed>(reg))
  {
    if (reg.is_entity_dying(i) || follower.lost_target) {
      continue;
    }
    std::size_t target_id = follower.target;

    if (target_id == i) {
      follower.lost_target = true;
      this->_event_manager.get().emit<ComponentBuilder>(
          i,
          this->_registry.get().get_component_key<Follower>(),
          follower.to_bytes());
      continue;
    }

    if (reg.is_entity_dying(target_id)
        || !reg.has_component<Position>(target_id))
    {
      follower.lost_target = true;

      this->_event_manager.get().emit<ComponentBuilder>(
          i,
          this->_registry.get().get_component_key<Follower>(),
          follower.to_bytes());
      continue;
    }

    Vector2D target_position = positions[target_id].value().pos;
    Vector2D vect = target_position - position.pos;

    Vector2D new_direction = vect.normalize();

    Vector2D direction_diff = new_direction - direction.direction;
    double direction_change = direction_diff.length();

    if (direction_change > DIRECTION_TOLERANCE) {
      direction.direction = new_direction;
      if (reg.has_component<Facing>(i)) {
        faces[i]->direction = new_direction;
        this->_event_manager.get().emit<ComponentBuilder>(
            i,
            this->_registry.get().get_component_key<Facing>(),
            faces[i]->to_bytes());
      }

      this->_event_manager.get().emit<ComponentBuilder>(
          i,
          this->_registry.get().get_component_key<Direction>(),
          direction.to_bytes());
    }
  }
}

void Target::on_interaction_zone(const InteractionZoneEvent& event)
{
  const auto& positions = this->_registry.get().get_components<Position>();
  auto& followers = this->_registry.get().get_components<Follower>();
  const auto& teams = this->_registry.get().get_components<Team>();

  if (!this->_registry.get().has_component<Follower>(event.source)
      || !followers[event.source]->lost_target)
  {
    return;
  }

  std::optional<Registry::Entity> closest_entity = std::nullopt;
  double closest_distance_sq = event.radius * event.radius;

  for (const Registry::Entity& candidate : event.candidates) {
    if (!this->_registry.get().has_component<Health>(candidate)) {
      continue;
    }
    if (this->_registry.get().has_component<Team>(candidate)
        && this->_registry.get().has_component<Team>(event.source)
        && teams[candidate]->name == teams[event.source]->name)
    {
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
  if (closest_entity.has_value()
      && closest_entity != followers[event.source]->target)
  {
    followers[event.source]->target = closest_entity.value();
    followers[event.source]->lost_target = false;

    this->_event_manager.get().emit<ComponentBuilder>(
        event.source,
        this->_registry.get().get_component_key<Follower>(),
        followers[event.source]->to_bytes());
  }
}

extern "C"
{
void* entry_point(Registry& r, EventManager& em, EntityLoader& e)
{
  return new Target(r, em, e);
}
}
