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

class TurretPattern : public MovementPattern
{
public:
  void update(Registry::Entity entity,
              Registry& registry,
              EventManager& em,
              MovementBehavior& /*behavior*/,
              Position& /*pos*/,
              Direction& direction,
              Speed& speed,
              double /*dt*/) override
  {
    Vector2D default_speed(0.0, 0.0);
    Vector2D default_direction(0.0, 0.0);

    if (speed.speed != default_speed) {
      speed.speed.x = 0.0;
      speed.speed.y = 0.0;
      em.emit<ComponentBuilder>(
          entity, registry.get_component_key<Speed>(), speed.to_bytes());
    }
    if (direction.direction != default_direction) {
      direction.direction.x = 0.0;
      direction.direction.y = 0.0;
      em.emit<ComponentBuilder>(entity,
                                registry.get_component_key<Direction>(),
                                direction.to_bytes());
    }
  }
};
