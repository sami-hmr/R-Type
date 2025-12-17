#pragma once

#include "ecs/Registry.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/components/AttackBehavior.hpp"
#include "plugin/components/Position.hpp"
#include "plugin/components/Velocity.hpp"

class AttackPattern
{
public:
  virtual ~AttackPattern() = default;

  virtual void execute(Registry::Entity entity,
                       Registry& registry,
                       AttackBehavior& behavior,
                       Position& pos,
                       Velocity& vel,
                       double dt) = 0;
};
