#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "libs/Rect.hpp"

class ICollisionAlgorithm
{
public:
  struct CollisionEntity
  {
    size_t entity_id;
    Rect bounds;
  };

  struct CollisionPair
  {
    size_t entity_a;
    size_t entity_b;
  };

  virtual ~ICollisionAlgorithm() = default;

  virtual void update(std::vector<CollisionEntity> const& entities) = 0;

  virtual std::vector<CollisionPair> detect_collisions(
      std::vector<CollisionEntity> const& entities) = 0;
  virtual std::vector<CollisionEntity> detect_range_collisions(
      Rect const& range) = 0;
  virtual std::string get_name() const = 0;
};
