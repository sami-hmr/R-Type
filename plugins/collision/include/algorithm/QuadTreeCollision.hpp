#pragma once

#include "QuadTreeNode.hpp"
#include "ICollisionAlgorithm.hpp"

class QuadTreeCollision : public ICollisionAlgorithm
{
public:
  QuadTreeCollision();
  QuadTreeCollision(double world_width, double world_height);
  ~QuadTreeCollision() override = default;

  std::vector<CollisionPair> detect_collisions(
      SparseArray<Position> const& positions,
      SparseArray<Collidable> const& collidables) override;

  void update(SparseArray<Position> const& positions,
              SparseArray<Collidable> const& collidables) override;

  void set_world_bounds(double width, double height);

private:
  bool check_collision(QuadTreeEntity const& a, QuadTreeEntity const& b) const;

  std::unique_ptr<QuadTreeNode> _root;
  double _world_width;
  double _world_height;
};
