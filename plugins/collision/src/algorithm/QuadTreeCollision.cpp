#include "algorithm/QuadTreeCollision.hpp"

#include "ICollisionAlgorithm.hpp"

void QuadTreeCollision::update(std::vector<CollisionEntity> const& entities)
{
  _root.clear();

  for (auto const& entity : entities) {
    CollisionEntity qt_entity(entity);
    _root.insert(qt_entity);
  }
}

std::vector<ICollisionAlgorithm::CollisionPair>
QuadTreeCollision::detect_collisions(
    std::vector<CollisionEntity> const& entities)
{
  std::vector<CollisionPair> collisions;

  for (auto const& entity : entities) {
    std::vector<CollisionEntity> candidates;
    _root.retrieve(candidates, entity.bounds);

    for (auto const& candidate : candidates) {
      if (candidate.entity_id <= entity.entity_id) {
        continue;
      }

      if (entity.bounds.intersects(candidate.bounds)) {
        collisions.push_back(CollisionPair {.entity_a = entity.entity_id,
                                            .entity_b = candidate.entity_id});
      }
    }
  }

  return collisions;
}

std::vector<ICollisionAlgorithm::CollisionEntity> QuadTreeCollision::detect_range_collisions(Rect const& range)
{
  std::vector<CollisionEntity> entities;

  _root.retrieve(entities, range);
  return entities;
}
