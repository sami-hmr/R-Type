#pragma once

#include <memory>
#include <vector>

#include "ICollisionAlgorithm.hpp"
#include "Rect.hpp"

class QuadTreeNode
{
public:
  static constexpr int max_entities = 10;
  static constexpr int max_levels = 5;

  QuadTreeNode(int level, Rect bounds)
      : bounds(bounds)
      , _level(level) {};
  ~QuadTreeNode() = default;

  void clear();
  void split();
  int get_index(Rect const& bounds) const;
  void insert(ICollisionAlgorithm::CollisionEntity const& entity);
  std::vector<ICollisionAlgorithm::CollisionEntity> retrieve(
      std::vector<ICollisionAlgorithm::CollisionEntity>& return_entities, Rect const& rect) const;

  Rect bounds;

private:
  int _level;
  std::vector<ICollisionAlgorithm::CollisionEntity> _entities;
  std::unique_ptr<QuadTreeNode> _nodes[4];
};
