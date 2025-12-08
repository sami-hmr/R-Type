#pragma once

#include <string>
#include <vector>

#include "Json/JsonParser.hpp"
#include "ecs/Registry.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/components/Health.hpp"
#include "plugin/components/Damage.hpp"
#include "plugin/events/Events.hpp"
#include "plugin/components/Heal.hpp"

class Life : public APlugin
{
public:
  Life(Registry& r, EntityLoader& l);
  static constexpr double heal_cooldown = 0.5;
  static constexpr double damage_cooldown = 0.5;

private:
  void init_health(Registry::Entity entity, JsonObject const& obj);
  void init_damage(Registry::Entity entity, JsonObject const& obj);
  void init_heal(Registry::Entity entity, JsonObject const& obj);
  void init_team(Registry::Entity const &entity, JsonObject const& obj);

  void damage_entity(const CollisionEvent& event, SparseArray<Health> &healths);
  void heal_entity(const CollisionEvent& event, SparseArray<Health> &healths);

  void update_cooldowns(Registry& reg);

  void on_collision(const CollisionEvent& event);
  void on_damage(const DamageEvent& event);
  void on_heal(const HealEvent& event);
};
