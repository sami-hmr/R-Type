#pragma once

#include <cmath>
#include <numbers>

#include "MovementPattern.hpp"
#include "NetworkShared.hpp"
#include "ecs/EventManager.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/components/Direction.hpp"
#include "plugin/components/Follower.hpp"
#include "plugin/components/InteractionZone.hpp"
#include "plugin/components/MovementBehavior.hpp"
#include "plugin/components/Speed.hpp"

class ZigzagPattern : public MovementPattern
{
public:
  static constexpr double DEFAULT_INTERVAL = 1.0;
  static constexpr double DEFAULT_ANGLE = 45.0;

  void update(Ecs::Entity entity,
              Registry& registry,
              EventManager& em,
              MovementBehavior& behavior,
              Position& /*pos*/,
              Direction& direction,
              Speed& /*speed*/,
              double dt) override
  {
    MovementPattern::update_delta(registry, em, entity, behavior, dt);

    double switch_interval =
        get_value_copy<double>(registry, behavior.params, "interval")
            .value_or(DEFAULT_INTERVAL);
    double angle = get_value_copy<double>(registry, behavior.params, "angle")
                       .value_or(DEFAULT_ANGLE);
    int dir = static_cast<int>(behavior.movement_delta / switch_interval) % 2;
    double rad = angle * std::numbers::pi / 180.0;

    Vector2D new_direction;
    new_direction.x = -1.0;
    new_direction.y = dir == 0 ? std::tan(rad) : -std::tan(rad);
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
