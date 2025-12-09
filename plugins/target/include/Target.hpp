#pragma once

#include <string>
#include <vector>

#include "Json/JsonParser.hpp"
#include "ecs/Registry.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/components/Follower.hpp"
#include "plugin/components/InteractionZone.hpp"
#include "plugin/components/Position.hpp"
#include "plugin/components/Velocity.hpp"
#include "plugin/events/InteractionZoneEvent.hpp"

class Target : public APlugin
{
public:
  Target(Registry& r, EntityLoader& l);

private:
  void init_follower(Registry::Entity entity, JsonObject const& obj);
  void on_interaction_zone(const InteractionZoneEvent& event);

  void target_system(Registry& reg,
                     SparseArray<Follower>& followers,
                     const SparseArray<Position>& positions,
                     SparseArray<Velocity>& velocities);
};
