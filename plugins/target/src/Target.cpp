#include "Target.hpp"

#include "Json/JsonParser.hpp"
#include "ecs/Registery.hpp"
#include "ecs/SparseArray.hpp"
#include "ecs/zipper/ZipperIndex.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/components/Follower.hpp"
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
}

void Target::init_follower(Registery::Entity entity, JsonObject const& obj)
{
  auto const& target =
      get_value<int>(this->_registery.get(), obj, "target");

  if (!target || target.value() < 0) {
    std::cerr << "Error loading Position component: unexpected value type "
                 "(expected follower: unsigned int)\n";
    return;
  }

  this->_registery.get().emplace_component<Follower>(entity, target.value());
}

void Target::target_system(Registery& reg,
                           SparseArray<Follower>& followers,
                           const SparseArray<Position>& positions,
                           SparseArray<Velocity>& velocities)
{
  double dt = reg.clock().delta_seconds();

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

extern "C"
{
void* entry_point(Registery& r, EntityLoader& e)
{
  return new Target(r, e);
}
}
