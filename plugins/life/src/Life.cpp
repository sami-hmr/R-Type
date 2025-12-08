#include <algorithm>
#include <any>
#include <format>
#include <functional>
#include <iostream>
#include <optional>

#include "Life.hpp"

#include "Json/JsonParser.hpp"
#include "Logger.hpp"
#include "ecs/Registery.hpp"
#include "ecs/SparseArray.hpp"
#include "ecs/zipper/Zipper.hpp"
#include "ecs/zipper/ZipperIndex.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/components/Damage.hpp"
#include "plugin/components/Heal.hpp"
#include "plugin/components/Health.hpp"
#include "plugin/components/Team.hpp"
#include "plugin/events/Events.hpp"

Life::Life(Registery& r, EntityLoader& l)
    : APlugin(r,
              l,
              {"moving", "collision"},
              {COMP_INIT(Health, Health, init_health),
               COMP_INIT(Damage, Damage, init_damage),
               COMP_INIT(Heal, Heal, init_heal),
               COMP_INIT(Team, Team, init_team)})
{
  this->_registery.get().register_component<Health>();
  this->_registery.get().register_component<Damage>();
  this->_registery.get().register_component<Heal>();
  this->_registery.get().register_component<Team>();

  this->_registery.get().add_system<Health>(
      [this](Registery& r, const SparseArray<Health>&)
      { this->update_cooldowns(r); },
      2);

  this->_registery.get().on<DamageEvent>([this](const DamageEvent& event)
                                         { this->on_damage(event); });

  this->_registery.get().on<HealEvent>([this](const HealEvent& event)
                                       { this->on_heal(event); });
  this->_registery.get().on<CollisionEvent>([this](const CollisionEvent& event)
                                            { this->on_collision(event); });
}

void Life::init_health(Registery::Entity entity, JsonObject const& obj)
{
  auto const& current = get_value<int>(this->_registery.get(), obj, "current");
  auto const& max = get_value<int>(this->_registery.get(), obj, "max");

  if (!current || !max) {
    std::cerr << "Error loading health component: unexpected value type or "
                 "missing value in JsonObject\n";
    return;
  }
  this->_registery.get().emplace_component<Health>(
      entity, current.value(), max.value());
}

void Life::init_damage(Registery::Entity entity, JsonObject const& obj)
{
  auto const& value = get_value<int>(this->_registery.get(), obj, "amount");

  if (!value) {
    std::cerr << "Error loading damage component: unexpected value type or "
                     "missing value in JsonObject\n";
    return;
  }
  this->_registery.get().emplace_component<Damage>(entity, value.value());
}

void Life::init_heal(Registery::Entity entity, JsonObject const& obj)
{
  auto const& value = get_value<int>(this->_registery.get(), obj, "amount");

  if (!value) {
    std::cerr << "Error loading heal component: unexpected value type or "
                     "missing value in JsonObject\n";
    return;
  }
  this->_registery.get().emplace_component<Heal>(entity, value.value());
}

void Life::init_team(Registery::Entity const& entity, JsonObject const& obj)
{
  auto const& value =
      get_value<std::string>(this->_registery.get(), obj, "name");

  if (!value) {
    std::cerr << "Error loading team component: unexpected value type or "
                     "missing value in JsonObject\n";
    return;
  }
  this->_registery.get().emplace_component<Team>(entity, value.value());
}

void Life::damage_entity(const CollisionEvent& event,
                         SparseArray<Health>& healths)
{
  auto& damages = this->_registery.get().get_components<Damage>();

  if (healths[event.a]->damage_delta >= damage_cooldown) {
    healths[event.a]->damage_delta = 0.0;
    _registery.get().emit<DamageEvent>(
        event.a, event.b, damages[event.b]->amount);
  }
}

void Life::heal_entity(const CollisionEvent& event,
                       SparseArray<Health>& healths)
{
  auto& healers = this->_registery.get().get_components<Heal>();

  if (healths[event.a]->heal_delta >= heal_cooldown) {
    healths[event.a]->heal_delta = 0.0;
    _registery.get().emit<HealEvent>(
        event.a, event.b, healers[event.b]->amount);
  }
}

void Life::on_collision(const CollisionEvent& event)
{
  auto& healths = this->_registery.get().get_components<Health>();
  auto& teams = this->_registery.get().get_components<Team>();

  if (!this->_registery.get().has_component<Health>(event.a)
      || !this->_registery.get().has_component<Team>(event.a)
      || !this->_registery.get().has_component<Team>(event.b))
  {
    return;
  }

  if (this->_registery.get().has_component<Damage>(event.b)
      && teams[event.a]->name != teams[event.b]->name)
  {
    damage_entity(event, healths);
  } else if (this->_registery.get().has_component<Heal>(event.b)
             && teams[event.a]->name == teams[event.b]->name)
  {
    heal_entity(event, healths);
  }
}

void Life::on_damage(const DamageEvent& event)
{
  auto& healths = this->_registery.get().get_components<Health>();

  if (!this->_registery.get().has_component<Health>(event.target)) {
    return;
  }
  if (event.target < healths.size() && healths[event.target].has_value()) {
    healths[event.target]->current -= event.amount;
  } else {
    return;
  }
  this->_registery.get().emit<LogEvent>(
      "HealthSystem",
      LogLevel::INFO,
      std::format("Entity {} took {} damage from Entity {}",
                  event.target,
                  event.amount,
                  event.source));

  if (healths[event.target]->current <= 0
      && !this->_registery.get().is_entity_dying(event.target))
  {
    this->_registery.get().emit<LogEvent>(
        "HealthSystem",
        LogLevel::WARNING,
        std::format("Entity {} died!", event.target));

    this->_registery.get().kill_entity(event.target);
  }
}

void Life::on_heal(const HealEvent& event)
{
  auto& healths = this->_registery.get().get_components<Health>();

  if (!this->_registery.get().has_component<Health>(event.target)) {
    return;
  }
  if (event.target < healths.size() && healths[event.target].has_value()) {
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
    return;
  }
}

void Life::update_cooldowns(Registery& reg)
{
  double dt = reg.clock().delta_seconds();
  auto& healths = reg.get_components<Health>();

  for (auto&& [i, health] : ZipperIndex(healths)) {
    if (!reg.is_entity_dying(i)) {
      health.damage_delta += dt;
      health.heal_delta += dt;
    }
  }
}

extern "C"
{
void* entry_point(Registery& r, EntityLoader& e)
{
  return new Life(r, e);
}
}
