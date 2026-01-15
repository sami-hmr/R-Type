#pragma once

#include <vector>

#include "ICollisionAlgorithm.hpp"
#include "libs/Rect.hpp"

class QuadTreeNode
{
public:
  static constexpr int max_entities = 10;
  static constexpr int max_levels = 5;

  QuadTreeNode(int level, Rect bounds)
      : bounds(bounds)
      , _level(level) {};
  ~QuadTreeNode() = default;

  QuadTreeNode(const QuadTreeNode&) = default;
  QuadTreeNode& operator=(const QuadTreeNode&) = default;
  QuadTreeNode(QuadTreeNode&&) noexcept = default;
  QuadTreeNode& operator=(QuadTreeNode&&) noexcept = default;

  void clear();
  void split();
  int get_index(Rect const& bounds) const;
  void insert(ICollisionAlgorithm::CollisionEntity const& entity);
  std::vector<ICollisionAlgorithm::CollisionEntity> retrieve(
      std::vector<ICollisionAlgorithm::CollisionEntity>& return_entities,
      Rect const& rect) const;

  Rect bounds;

private:
  int _level;
  std::vector<ICollisionAlgorithm::CollisionEntity> _entities;
  std::vector<QuadTreeNode> _nodes;
};
