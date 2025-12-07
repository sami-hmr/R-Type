#include "Target.hpp"

#include "Json/JsonParser.hpp"
#include "ecs/Registery.hpp"
#include "ecs/SparseArray.hpp"
#include "ecs/zipper/ZipperIndex.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/components/Follower.hpp"
#include "plugin/components/InteractionZone.hpp"
#include "plugin/components/Position.hpp"
#include "plugin/components/Velocity.hpp"
#include "plugin/events/Events.hpp"

Target::Target(Registery& r, EntityLoader& l)
    : APlugin(r, l, {"moving"}, {COMP_INIT(Follower, Follower, init_follower)})
{
  this->_registery.get().register_component<Follower>();

  this->_registery.get().add_system<Follower, Position, Velocity>(
      [this](Registery& r,
             SparseArray<Follower>& followers,
             const SparseArray<Position>& positions,
             SparseArray<Velocity>& velocities)
      { this->target_system(r, followers, positions, velocities); });
  this->_registery.get().on<InteractionZoneEvent>(
      [this](const InteractionZoneEvent& event)
      { this->on_interaction_zone(event); });
}

void Target::init_follower(Registery::Entity entity, JsonObject const& obj)
{
  this->_registery.get().emplace_component<Follower>(entity);
}

void Target::target_system(Registery& reg,
                           SparseArray<Follower>& followers,
                           const SparseArray<Position>& positions,
                           SparseArray<Velocity>& velocities)
{
  for (auto&& [i, follower, position, velocity] :
       ZipperIndex(followers, positions, velocities))
  {
    if (reg.is_entity_dying(i) || follower.lost_target) {
      continue;
    }
    std::size_t target_id = follower.target;

    if (reg.is_entity_dying(target_id)
        || !reg.has_component<Position>(target_id))
    {
      follower.lost_target = true;
      continue;
    }

    Vector2D target_position = positions[follower.target].value().pos;
    Vector2D vect = target_position - position.pos;

    velocity.direction = vect.normalize();
  }
}

void Target::on_interaction_zone(const InteractionZoneEvent& event)
{
  const auto& positions = this->_registery.get().get_components<Position>();
  auto& followers = this->_registery.get().get_components<Follower>();

  if (!this->_registery.get().has_component<Follower>(event.source)
      || !followers[event.source]->lost_target)
  {
    return;
  }

  std::optional<Registery::Entity> closest_entity = std::nullopt;
  double closest_distance_sq = event.radius * event.radius;

  for (const Registery::Entity& candidate : event.candidates) {
    Vector2D distance =
        positions[candidate]->pos - positions[event.source]->pos;
    double distance_sq = distance.length();

    if (distance_sq < closest_distance_sq) {
      closest_distance_sq = distance_sq;
      closest_entity = candidate;
    }
  }
  if (closest_entity.has_value()) {
    followers[event.source] = closest_entity;
  }
}

extern "C"
{
void* entry_point(Registery& r, EntityLoader& e)
{
  return new Target(r, e);
}
}
