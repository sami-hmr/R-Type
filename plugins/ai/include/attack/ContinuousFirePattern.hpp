#pragma once

#include <cmath>

#include "AttackPattern.hpp"
#include "Json/JsonParser.hpp"
#include "ecs/EventManager.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/components/BasicWeapon.hpp"
#include "plugin/components/Direction.hpp"
#include "plugin/components/Speed.hpp"
#include "plugin/events/WeaponEvent.hpp"

class ContinuousFirePattern : public AttackPattern
{
public:
  static constexpr double DEFAULT_INTERVAL = 2.0;

  void execute(Registry::Entity entity,
               Registry&  /*registry*/,
               EventManager& em,
               AttackBehavior&  /*behavior*/,
               Position& /*pos*/,
               Direction& /*dir*/,
               Speed& /*speed*/,
               double  /*dt*/) override
  {
    em.emit<FireBullet>(entity);
  }
};
