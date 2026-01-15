#pragma once

#include "ecs/EventManager.hpp"
#include "ecs/Registry.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/components/Direction.hpp"
#include "plugin/components/MovementBehavior.hpp"
#include "plugin/components/Position.hpp"
#include "plugin/components/Speed.hpp"

class MovementPattern
{
public:
  virtual ~MovementPattern() = default;

  static constexpr double DIRECTION_TOLERANCE = 0.1;
  static constexpr double DELTA_TOLERANCE = 0.5;

  virtual void update(Registry::Entity entity,
                      Registry& registry,
                      EventManager& em,
                      MovementBehavior& behavior,
                      Position& pos,
                      Direction& dir,
                      Speed& speed,
                      double dt) = 0;

  static Vector2D get_origin(Registry& registry, MovementBehavior& behavior)
  {
    if (behavior.params.contains("origin")) {
      auto origin_opt =
          get_value_copy<Vector2D>(registry, behavior.params, "origin");
      if (origin_opt) {
        return origin_opt.value();
      }
    }
    if (behavior.params.contains("target_id")) {
      auto target_id =
          get_value_copy<int>(registry, behavior.params, "target_id");
      if (target_id.has_value()
          && registry.has_component<Position>(target_id.value()))
      {
        return registry.get_components<Position>()[target_id.value()]
            .value()
            .pos;
      }
    }
    return Vector2D(0.0, 0.0);
  }

  static void update_delta(Registry& registry,
                           EventManager& em,
                           Registry::Entity const& entity,
                           MovementBehavior& behavior,
                           double const& dt)
  {
    auto const& time = registry.clock().now().time_since_epoch();
    auto const& duration = std::chrono::duration<double>(time).count();

    behavior.movement_delta += dt;
    if ((duration - behavior.last_update) > DELTA_TOLERANCE) {
      behavior.last_update = duration;
      em.emit<ComponentBuilder>(entity,
                                registry.get_component_key<MovementBehavior>(),
                                behavior.to_bytes());
    }
  }
};
