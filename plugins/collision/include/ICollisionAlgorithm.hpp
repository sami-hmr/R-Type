#pragma once

#include <string>
#include <vector>

#include "ecs/Registery.hpp"
#include "ecs/SparseArray.hpp"
#include "plugin/components/Collidable.hpp"
#include "plugin/components/Position.hpp"

class ICollisionAlgorithm
{
public:
  virtual ~ICollisionAlgorithm() = default;

  struct CollisionPair
  {
    Registery::Entity entity_a;
    Registery::Entity entity_b;
  };

  virtual std::vector<CollisionPair> detect_collisions(
      SparseArray<Position> const& positions,
      SparseArray<Collidable> const& collidables) = 0;

  virtual void update(SparseArray<Position> const& positions,
                      SparseArray<Collidable> const& collidables) = 0;
};
