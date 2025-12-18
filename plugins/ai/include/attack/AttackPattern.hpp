#pragma once

#include "ecs/Registry.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/components/AttackBehavior.hpp"
#include "plugin/components/Direction.hpp"
#include "plugin/components/Position.hpp"
#include "plugin/components/Direction.hpp"
#include "plugin/components/Speed.hpp"

class AttackPattern
{
public:
  virtual ~AttackPattern() = default;

  virtual void execute(Registry::Entity entity,
                       Registry& registry,
                       AttackBehavior& behavior,
                       Position& pos,
                       Direction& dir,
                       Speed& speed,
                       double dt) = 0;
};
