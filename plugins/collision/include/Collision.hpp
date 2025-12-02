#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Events.hpp"
#include "ICollisionAlgorithm.hpp"
#include "Json/JsonParser.hpp"
#include "ecs/Registery.hpp"
#include "ecs/SparseArray.hpp"
#include "ecs/zipper/Zipper.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/components/Collidable.hpp"
#include "plugin/components/Position.hpp"
#include "plugin/components/Velocity.hpp"

class Collision : public APlugin
{
public:
  Collision(Registery& r, EntityLoader& l);

  void set_algorithm(std::unique_ptr<ICollisionAlgorithm> algo);

private:
  void init_collision(Registery::Entity const &entity,
                     JsonVariant const& config);

  void collision_system(Registery& r,
                       SparseArray<Position> positions,
                       SparseArray<Collidable> collidables);

  std::unique_ptr<ICollisionAlgorithm> _collision_algo;
  const std::vector<std::string> depends_on = {"moving"};
};
