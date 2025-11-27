#pragma once

#include <string>
#include <vector>

#include "Json/JsonParser.hpp"
#include "ecs/Registery.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"

/**
 * @brief Component storing entity health values.
 */
struct HealthComponent
{
  int current;
  int max;
};

/**
 * @brief Event representing damage dealt to an entity.
 */
struct DamageEvent
{
  Registery::Entity target;
  Registery::Entity source;
  int amount;
  std::string damage_type;
};

/**
 * @brief Event representing healing applied to an entity.
 */
struct HealEvent
{
  Registery::Entity target;
  int amount;
};

/**
 * @brief Event triggered when an entity dies.
 */
struct DeathEvent
{
  Registery::Entity entity;
  std::string entity_name;
};

/**
 * @brief Plugin managing health components and related events.
 */
class Health : public APlugin
{
public:
  Health(Registery& r, EntityLoader& l);

private:
  void init_health(Registery::Entity entity, JsonVariant const& config);

  void on_damage(const DamageEvent& event);
  void on_heal(const HealEvent& event);

  const std::vector<std::string> depends_on;
};
