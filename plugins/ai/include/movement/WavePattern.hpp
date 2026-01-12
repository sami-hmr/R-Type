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

class WavePattern : public MovementPattern
{
public:
  static constexpr double DEFAULT_AMPLITUDE = 0.7;
  static constexpr double DEFAULT_FREQUENCY = 2.0;

  void update(Registry::Entity entity,
              Registry& registry,
              EventManager& em,
              MovementBehavior& behavior,
              Position& /*pos*/,
              Direction& direction,
              Speed& /*speed*/,
              double dt) override
  {
    behavior.movement_delta += dt;
    em.emit<ComponentBuilder>(entity,
                              registry.get_component_key<MovementBehavior>(),
                              behavior.to_bytes());

    double amplitude =
        get_value_copy<double>(registry, behavior.params, "amplitude")
            .value_or(DEFAULT_AMPLITUDE);
    double frequency =
        get_value_copy<double>(registry, behavior.params, "frequency")
            .value_or(DEFAULT_FREQUENCY);

    Vector2D new_direction;
    new_direction.x = -1.0;
    new_direction.y = std::sin(behavior.movement_delta * frequency) * amplitude;
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
