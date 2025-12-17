#pragma once

#include <cmath>

#include "AttackPattern.hpp"
#include "Json/JsonParser.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/components/BasicWeapon.hpp"
#include "plugin/events/WeaponEvent.hpp"

class ContinuousFirePattern : public AttackPattern
{
public:
  void execute(Registry::Entity entity,
               Registry& registry,
               AttackBehavior& behavior,
               Position& pos,
               Velocity& vel,
               double dt) override
  {
    behavior.attack_delta += dt;

    registry.emit<ComponentBuilder>(
      entity,
      registry.get_component_key<AttackBehavior>(),
      behavior.to_bytes());

    if (behavior.attack_delta >= behavior.attack_interval) {
      behavior.attack_delta = 0.0;

      auto& weapons = registry.get_components<BasicWeapon>();
      if (weapons[entity].has_value()) {
        registry.emit<FireBullet>(entity);
      }
    }
  }
};
