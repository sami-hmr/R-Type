#pragma once

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
  
  virtual void update(Registry::Entity entity,
                     Registry& registry,
                     MovementBehavior& behavior,
                     Position& pos,
                     Direction& dir,
                     Speed& speed,
                     double dt) = 0;
};
