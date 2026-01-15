#pragma once

#include <cmath>

#include "MovementPattern.hpp"
#include "NetworkShared.hpp"
#include "ecs/EventManager.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/components/Direction.hpp"
#include "plugin/components/Follower.hpp"
#include "plugin/components/InteractionZone.hpp"
#include "plugin/components/MovementBehavior.hpp"
#include "plugin/components/Speed.hpp"

class StraightPattern : public MovementPattern
{
public:
  void update(Registry::Entity entity,
              Registry& registry,
              EventManager& em,
              MovementBehavior& /*behavior*/,
              Position& /*pos*/,
              Direction& direction,
              Speed& /*speed*/,
              double /*dt*/) override
  {
    Vector2D new_direction(-1.0, 0.0);
    new_direction = new_direction.normalize();

    Vector2D direction_diff = new_direction - direction.direction;
    double direction_change = direction_diff.length();
    if (direction_change > DIRECTION_TOLERANCE) {
      direction.direction = new_direction;
      em.emit<ComponentBuilder>(entity,
                                registry.get_component_key<Direction>(),
                                direction.to_bytes());
    }
  }
};
