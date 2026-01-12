#pragma once

#include "ecs/EventManager.hpp"
#include "ecs/Registry.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/components/MovementBehavior.hpp"
#include "plugin/components/Position.hpp"
#include "plugin/components/Direction.hpp"
#include "plugin/components/Speed.hpp"

class MovementPattern
{
public:
  virtual ~MovementPattern() = default;

  static constexpr double DIRECTION_TOLERANCE = 0.1;

  virtual void update(Registry::Entity entity,
                     Registry& registry,
                     EventManager &em,
                     MovementBehavior& behavior,
                     Position& pos,
                     Direction& dir,
                     Speed& speed,
                     double dt) = 0;
};
