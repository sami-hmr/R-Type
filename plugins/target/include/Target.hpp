#pragma once

#include <string>
#include <vector>

#include "Json/JsonParser.hpp"
#include "ecs/Registery.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/components/Position.hpp"
#include "plugin/components/Follower.hpp"
#include "plugin/components/Velocity.hpp"
#include "plugin/events/Events.hpp"

class Target : public APlugin
{
public:
  Target(Registery& r, EntityLoader& l);

private:
  void init_follower(Registery::Entity entity, JsonObject const& obj);

  void target_system(Registery& reg, SparseArray<Follower>& followers, const SparseArray<Position>& positions, SparseArray<Velocity>& velocities);
};
