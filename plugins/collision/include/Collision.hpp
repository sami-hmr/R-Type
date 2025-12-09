#pragma once

#include <memory>
#include <string>
#include <vector>

#include "ICollisionAlgorithm.hpp"
#include "Json/JsonParser.hpp"
#include "ecs/Registry.hpp"
#include "ecs/SparseArray.hpp"
#include "ecs/zipper/Zipper.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/components/Collidable.hpp"
#include "plugin/components/InteractionZone.hpp"
#include "plugin/components/Position.hpp"
#include "plugin/components/Velocity.hpp"
#include "plugin/events/EntityEvents.hpp"
#include "plugin/events/Events.hpp"

class Collision : public APlugin
{
public:
  Collision(Registry& r, EntityLoader& l);

  void set_algorithm(std::unique_ptr<ICollisionAlgorithm> algo);

private:
  void init_collision(Registry::Entity const& entity, JsonObject const& obj);
  void init_interaction_zone(Registry::Entity const& entity,
                             JsonObject const& obj);

  void collision_system(Registry& r,
                        const SparseArray<Position>& positions,
                        const SparseArray<Collidable>& collidables);
  void interaction_zone_system(Registry& r,
                               const SparseArray<Position>& positions,
                               const SparseArray<InteractionZone>& zones);
  void on_collision(const CollisionEvent& c);

  std::unique_ptr<ICollisionAlgorithm> _collision_algo;
};
