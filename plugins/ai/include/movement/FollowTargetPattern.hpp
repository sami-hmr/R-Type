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

class FollowTargetPattern : public MovementPattern
{
public:
  static constexpr double DEFAULT_RADIUS = 1.5;

  void update(Ecs::Entity entity,
              Registry& registry,
              EventManager& em,
              MovementBehavior& behavior,
              Position& /*pos*/,
              Direction& direction,
              Speed& /*speed*/,
              double /*dt*/) override
  {
    double radius = get_value_copy<double>(registry, behavior.params, "radius")
                        .value_or(DEFAULT_RADIUS);

    if (!registry.has_component<Follower>(entity)) {
      registry.add_component(entity, Follower());
      direction.direction.x = -1.0;
      direction.direction.y = 0.0;
      em.emit<ComponentBuilder>(entity,
                                registry.get_component_key<Direction>(),
                                direction.to_bytes());
    }
    if (!registry.has_component<InteractionZone>(entity)) {
      registry.add_component(entity, InteractionZone(radius));
      direction.direction.x = -1.0;
      direction.direction.y = 0.0;
      em.emit<ComponentBuilder>(entity,
                                registry.get_component_key<Direction>(),
                                direction.to_bytes());
    }
  }
};
