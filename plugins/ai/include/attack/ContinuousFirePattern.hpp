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
               Registry& registry,
               EventManager& em,
               AttackBehavior& behavior,
               Position& /*pos*/,
               Direction& /*dir*/,
               Speed& /*speed*/,
               double dt) override
  {
    behavior.attack_delta += dt;

    em.emit<ComponentBuilder>(entity,
                              registry.get_component_key<AttackBehavior>(),
                              behavior.to_bytes());

    double attack_interval =
        get_value_copy<double>(registry, behavior.params, "attack_interval")
            .value_or(DEFAULT_INTERVAL);

    if (behavior.attack_delta >= attack_interval) {
      behavior.attack_delta = 0.0;

      em.emit<ComponentBuilder>(entity,
                                registry.get_component_key<AttackBehavior>(),
                                behavior.to_bytes());

      em.emit<FireBullet>(entity);
    }
  }
};
