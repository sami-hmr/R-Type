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

class CirclePattern : public MovementPattern
{
public:
  static constexpr double DEFAULT_RADIUS = 100.0;
  static constexpr double DEFAULT_ANGULAR_SPEED = 1.5;

  void update(Registry::Entity entity,
              Registry& registry,
              EventManager& em,
              MovementBehavior& behavior,
              Position& pos,
              Direction& direction,
              Speed& /*speed*/,
              double dt) override

  {
    behavior.movement_delta += dt;
    em.emit<ComponentBuilder>(entity,
                              registry.get_component_key<MovementBehavior>(),
                              behavior.to_bytes());

    double radius = get_value_copy<double>(registry, behavior.params, "radius")
                        .value_or(DEFAULT_RADIUS);
    double angular_speed =
        get_value_copy<double>(registry, behavior.params, "angular_speed")
            .value_or(DEFAULT_ANGULAR_SPEED);

    double angle = behavior.movement_delta * angular_speed;

    Vector2D target_pos;
    target_pos.x = std::cos(angle) * radius;
    target_pos.y = std::sin(angle) * radius;

    Vector2D to_target = target_pos - pos.pos;
    if (to_target.length() > 0.1) {
      Vector2D new_direction = to_target.normalize();
      Vector2D direction_diff = new_direction - direction.direction;
      double direction_change = direction_diff.length();

      if (direction_change > DIRECTION_TOLERANCE) {
        direction.direction = new_direction;
        em.emit<ComponentBuilder>(entity,
                                  registry.get_component_key<Direction>(),
                                  direction.to_bytes());
      }
    }
  }
};
