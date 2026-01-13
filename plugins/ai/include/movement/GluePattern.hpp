#pragma once

#include <cmath>

#include "MovementPattern.hpp"
#include "NetworkShared.hpp"
#include "ecs/EventManager.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/components/Direction.hpp"
#include "plugin/components/InteractionZone.hpp"
#include "plugin/components/MovementBehavior.hpp"
#include "plugin/components/Parasite.hpp"
#include "plugin/components/Speed.hpp"

class GluePattern : public MovementPattern
{
public:
  static constexpr double DEFAULT_RADIUS = 0.3;
  static constexpr std::string DEFAULT_BEHAVIOR = "glue";

  void update(Registry::Entity entity,
              Registry& registry,
              EventManager& em,
              MovementBehavior& behavior,
              Position& /*pos*/,
              Direction& direction,
              Speed& speed,
              double /*dt*/) override
  {
    auto target_id =
        get_value_copy<int>(registry, behavior.params, "target_id");
    double radius = get_value_copy<int>(registry, behavior.params, "radius")
                        .value_or(DEFAULT_RADIUS);
    std::string parasite_behavior = get_value_copy<std::string>(registry, behavior.params, "behavior")
                        .value_or(DEFAULT_BEHAVIOR);

    if (!target_id.has_value()) {
      if (!registry.has_component<Parasite>(entity)) {
        registry.add_component(entity, Parasite(parasite_behavior));
      }
      if (!registry.has_component<InteractionZone>(entity)) {
        registry.add_component(entity, InteractionZone(radius));
      }
    } else {
      if (registry.has_component<Direction, Speed>(target_id.value())) {
        auto& new_direction =
            registry.get_components<Direction>()[target_id.value()]
                .value()
                .direction;
        Vector2D direction_diff = new_direction - direction.direction;
        double direction_change = direction_diff.length();

        auto& new_speed =
            registry.get_components<Speed>()[target_id.value()].value().speed;
        Vector2D speed_diff = new_speed - speed.speed;
        double speed_change = speed_diff.length();

        if (direction_change > DIRECTION_TOLERANCE) {
          direction.direction = new_direction;
          em.emit<ComponentBuilder>(entity,
                                    registry.get_component_key<Direction>(),
                                    direction.to_bytes());
        }

        if (speed_change > DIRECTION_TOLERANCE) {
          speed.speed = new_speed;
          em.emit<ComponentBuilder>(
              entity, registry.get_component_key<Speed>(), speed.to_bytes());
        }
      }
    }
  }
};
