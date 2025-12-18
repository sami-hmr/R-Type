#pragma once

#include "ICollisionAlgorithm.hpp"
#include "QuadTreeNode.hpp"

class QuadTreeCollision : public ICollisionAlgorithm
{
public:
  QuadTreeCollision(double width, double height)
      : _root(0, Rect {.x = 0.0, .y = 0.0, .width = width, .height = height})
  {
  }

  void update(std::vector<CollisionEntity> const& entities) override;
  std::vector<CollisionPair> detect_collisions(
      std::vector<CollisionEntity> const& entities) override;

  std::vector<CollisionEntity> detect_range_collisions(
      Rect const& range) override;
  std::string get_name() const override { return "QuadTree"; }

private:
  QuadTreeNode _root;
};
