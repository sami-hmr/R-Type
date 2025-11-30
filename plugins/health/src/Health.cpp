#include <algorithm>
#include <format>
#include <iostream>

#include "Health.hpp"

#include "Logger.hpp"
#include "ecs/Registery.hpp"
#include "plugin/EntityLoader.hpp"

Health::Health(Registery& r, EntityLoader& l)
    : APlugin(r, l, {"moving"}, {COMP_INIT(health, init_health)})
{
  this->_registery.get().register_component<HealthComponent>("health:health");

  this->_registery.get().on<DamageEvent>([this](const DamageEvent& event)
                                         { this->on_damage(event); });

  this->_registery.get().on<HealEvent>([this](const HealEvent& event)
                                       { this->on_heal(event); });
}

void Health::init_health(Registery::Entity entity, JsonVariant const& config)
{
  try {
    JsonObject obj = std::get<JsonObject>(config);
    int current = std::get<int>(obj.at("current").value);
    int max = std::get<int>(obj.at("max").value);

    this->_registery.get().emplace_component<HealthComponent>(
        entity, current, max);
  } catch (std::bad_variant_access const&) {
    std::cerr << "Error loading health component: unexpected value type\n";
  } catch (std::out_of_range const&) {
    std::cerr
        << "Error loading health component: missing value in JsonObject\n";
  }
}

void Health::on_damage(const DamageEvent& event)
{
  auto& healths = this->_registery.get().get_components<HealthComponent>();

  if (!healths[event.target].has_value()) {
    return;
  }

  healths[event.target]->current -= event.amount;

  this->_registery.get().emit<LogEvent>(
      "HealthSystem",
      LogLevel::INFO,
      std::format("Entity {} took {} {} damage from Entity {}",
                  event.target,
                  event.amount,
                  event.damage_type,
                  event.source));

  if (healths[event.target]->current <= 0) {
    this->_registery.get().emit<LogEvent>(
        "HealthSystem",
        LogLevel::WARNING,
        std::format("Entity {} died!", event.target));

    this->_registery.get().emit<DeathEvent>(
        event.target, std::format("Entity_{}", event.target));
  }
}

void Health::on_heal(const HealEvent& event)
{
  auto& healths = this->_registery.get().get_components<HealthComponent>();

  if (!healths[event.target].has_value()) {
    return;
  }

  int old_health = healths[event.target]->current;
  healths[event.target]->current =
      std::min(healths[event.target]->current + event.amount,
               healths[event.target]->max);

  int actual_heal = healths[event.target]->current - old_health;

  this->_registery.get().emit<LogEvent>(
      "HealthSystem",
      LogLevel::INFO,
      std::format("Entity {} healed for {} HP (now {}/{})",
                  event.target,
                  actual_heal,
                  healths[event.target]->current,
                  healths[event.target]->max));
}

extern "C"
{
void* entry_point(Registery& r, EntityLoader& e)
{
  return new Health(r, e);
}
}
