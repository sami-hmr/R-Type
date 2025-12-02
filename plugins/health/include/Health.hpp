#pragma once

#include <string>
#include <vector>

#include "ByteParser/ByteParser.hpp"
#include "Json/JsonParser.hpp"
#include "ecs/Registery.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/Byte.hpp"
#include "plugin/EntityLoader.hpp"

struct HealthComponent
{
  HealthComponent() = default;

  HealthComponent(int c, int m)
      : current(c)
      , max(m)
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(HealthComponent,
                           ([](int c, int m)
                            { return (HealthComponent) {c, m}; }),
                           parseByte<int>(),
                           parseByte<int>())
  DEFAULT_SERIALIZE(type_to_byte(this->current), type_to_byte(this->max))

  int current = 0;
  int max = 0;
};

struct DamageEvent
{
  Registery::Entity target;
  Registery::Entity source;
  int amount;
  std::string damage_type;
};

struct HealEvent
{
  Registery::Entity target;
  int amount;
};

struct DeathEvent
{
  Registery::Entity entity;
  std::string entity_name;
};

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
