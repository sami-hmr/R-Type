#pragma once

#include "ecs/Registry.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/components/MovementBehavior.hpp"
#include "plugin/components/Position.hpp"
#include "plugin/components/Velocity.hpp"

class MovementPattern
{
public:
  virtual ~MovementPattern() = default;
  
  virtual void update(Registry::Entity entity,
                     Registry& registry,
                     MovementBehavior& behavior,
                     Position& pos,
                     Velocity& vel,
                     double dt) = 0;
};
