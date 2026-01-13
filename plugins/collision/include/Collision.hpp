#pragma once

#include <memory>
#include <string>
#include <vector>

#include "ICollisionAlgorithm.hpp"
#include "Json/JsonParser.hpp"
#include "ecs/EventManager.hpp"
#include "ecs/Registry.hpp"
#include "ecs/SparseArray.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/components/Collidable.hpp"
#include "plugin/components/InteractionZone.hpp"
#include "plugin/components/Position.hpp"
#include "plugin/events/CollisionEvent.hpp"

class Collision : public APlugin
{
public:
  Collision(Registry& r, EventManager& em, EntityLoader& l);

  void set_algorithm(std::unique_ptr<ICollisionAlgorithm> algo);

private:
  void init_collision(Registry::Entity const& entity, JsonObject const& obj);
  void init_interaction_zone(Registry::Entity const& entity,
                             JsonObject const& obj);

  void init_interaction_borders(Registry::Entity const& entity,
                                           JsonObject const& obj);
  void collision_system(Registry& r);
  void interaction_zone_system(Registry& r);
  void on_collision(const CollisionEvent& c);

  std::unique_ptr<ICollisionAlgorithm> _collision_algo;
};
