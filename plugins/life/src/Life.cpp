#include <algorithm>
#include <any>
#include <format>
#include <functional>
#include <iostream>
#include <optional>

#include "Life.hpp"

#include "Json/JsonParser.hpp"
#include "Logger.hpp"
#include "NetworkShared.hpp"
#include "ecs/Registry.hpp"
#include "ecs/SparseArray.hpp"
#include "ecs/zipper/ZipperIndex.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/components/AnimatedSprite.hpp"
#include "plugin/components/Damage.hpp"
#include "plugin/components/Heal.hpp"
#include "plugin/components/Health.hpp"
#include "plugin/components/Team.hpp"
#include "plugin/events/CameraEvents.hpp"
#include "plugin/events/DamageEvent.hpp"
#include "plugin/events/HealEvent.hpp"

Life::Life(Registry& r, EntityLoader& l)
    : APlugin("life",
              r,
              l,
              {"moving", "collision"},
              {COMP_INIT(Health, Health, init_health),
               COMP_INIT(Damage, Damage, init_damage),
               COMP_INIT(Heal, Heal, init_heal),
               COMP_INIT(Team, Team, init_team)})
{
  REGISTER_COMPONENT(Health)
  REGISTER_COMPONENT(Damage)
  REGISTER_COMPONENT(Heal)
  REGISTER_COMPONENT(Team)
  this->_registry.get().add_system([this](Registry& r) { this->update_cooldowns(r); }, 2);


  this->_registry.get().on<DamageEvent>("DamageEvent",
                                        [this](const DamageEvent& event)
                                        { this->on_damage(event); });

  SUBSCRIBE_EVENT(DamageEvent, { this->on_damage(event); })
  SUBSCRIBE_EVENT(HealEvent, { this->on_heal(event); })
  SUBSCRIBE_EVENT(CollisionEvent, { this->on_collision(event); })
}

void Life::init_health(Registry::Entity entity, JsonObject const& obj)
{
  auto const& current =
      get_value<Health, int>(this->_registry.get(), obj, entity, "current");
  auto const& max =
      get_value<Health, int>(this->_registry.get(), obj, entity, "max");

  if (!current || !max) {
    std::cerr << "Error loading health component: unexpected value type or "
                 "missing value in JsonObject\n";
    return;
  }
  this->_registry.get().emplace_component<Health>(
      entity, current.value(), max.value(), heal_cooldown, damage_cooldown);
}

void Life::init_damage(Registry::Entity entity, JsonObject const& obj)
{
  auto const& value =
      get_value<Damage, int>(this->_registry.get(), obj, entity, "amount");

  if (!value) {
    std::cerr << "Error loading damage component: unexpected value type or "
                     "missing value in JsonObject\n";
    return;
  }
  this->_registry.get().emplace_component<Damage>(entity, value.value());
}

void Life::init_heal(Registry::Entity entity, JsonObject const& obj)
{
  auto const& value =
      get_value<Heal, int>(this->_registry.get(), obj, entity, "amount");

  if (!value) {
    std::cerr << "Error loading heal component: unexpected value type or "
                     "missing value in JsonObject\n";
    return;
  }
  this->_registry.get().emplace_component<Heal>(entity, value.value());
}

void Life::init_team(Registry::Entity const& entity, JsonObject const& obj)
{
  auto const& value =
      get_value<Team, std::string>(this->_registry.get(), obj, entity, "name");

  if (!value) {
    std::cerr << "Error loading team component: unexpected value type or "
                     "missing value in JsonObject\n";
    return;
  }
  this->_registry.get().emplace_component<Team>(entity, value.value());
}

void Life::damage_entity(const CollisionEvent& event,
                         SparseArray<Health>& healths)
{
  auto& damages = this->_registry.get().get_components<Damage>();

  if (healths[event.a]->damage_delta >= damage_cooldown) {
    healths[event.a]->damage_delta = 0.0;
    this->_registry.get().emit<ComponentBuilder>(
        event.a,
        this->_registry.get().get_component_key<Health>(),
        healths[event.a]->to_bytes());

    _registry.get().emit<DamageEvent>(
        event.a, event.b, damages[event.b]->amount);

  }
}

void Life::heal_entity(const CollisionEvent& event,
                       SparseArray<Health>& healths)
{
  auto& healers = this->_registry.get().get_components<Heal>();

  if (healths[event.a]->heal_delta >= heal_cooldown) {
    healths[event.a]->heal_delta = 0.0;
    _registry.get().emit<HealEvent>(event.a, event.b, healers[event.b]->amount);

    this->_registry.get().emit<ComponentBuilder>(
        event.a,
        this->_registry.get().get_component_key<Health>(),
        healths[event.a]->to_bytes());
  }
}

void Life::on_collision(const CollisionEvent& event)
{
  auto& healths = this->_registry.get().get_components<Health>();
  auto& teams = this->_registry.get().get_components<Team>();

  if (!this->_registry.get().has_component<Health>(event.a)
      || !this->_registry.get().has_component<Team>(event.a)
      || !this->_registry.get().has_component<Team>(event.b))
  {
    return;
  }

  if (this->_registry.get().has_component<Damage>(event.b)
      && teams[event.a]->name != teams[event.b]->name)
  {
    damage_entity(event, healths);
  } else if (this->_registry.get().has_component<Heal>(event.b)
             && teams[event.a]->name == teams[event.b]->name)
  {
    heal_entity(event, healths);
  }
}

void Life::on_damage(const DamageEvent& event)
{
  auto& healths = this->_registry.get().get_components<Health>();

  if (!this->_registry.get().has_component<Health>(event.target)) {
    return;
  }
  if (event.target < healths.size() && healths[event.target].has_value()) {
    healths[event.target]->current -= event.amount;

    this->_registry.get().emit<ComponentBuilder>(
        event.target,
        this->_registry.get().get_component_key<Health>(),
        healths[event.target]->to_bytes());
  } else {
    return;
  }
  this->_registry.get().emit<LogEvent>(
      "HealthSystem",
      LogLevel::INFO,
      std::format("Entity {} took {} damage from Entity {}",
                  event.target,
                  event.amount,
                  event.source));

  if (healths[event.target]->current <= 0
      && !this->_registry.get().is_entity_dying(event.target))
  {
    this->_registry.get().emit<LogEvent>(
        "HealthSystem",
        LogLevel::WARNING,
        std::format("Entity {} died!", event.target));

    if (!this->_registry.get().has_component<AnimatedSprite>(event.target)) {
      this->_registry.get().kill_entity(event.target);
    }
  }
}

void Life::on_heal(const HealEvent& event)
{
  auto& healths = this->_registry.get().get_components<Health>();

  if (!this->_registry.get().has_component<Health>(event.target)) {
    return;
  }
  if (event.target < healths.size() && healths[event.target].has_value()) {
    int old_health = healths[event.target]->current;
    healths[event.target]->current =
        std::min(healths[event.target]->current + event.amount,
                 healths[event.target]->max);

    this->_registry.get().emit<ComponentBuilder>(
        event.target,
        this->_registry.get().get_component_key<Health>(),
        healths[event.target]->to_bytes());

    int actual_heal = healths[event.target]->current - old_health;

    this->_registry.get().emit<LogEvent>(
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

void Life::update_cooldowns(Registry& reg)
{
  double dt = reg.clock().delta_seconds();

  for (auto&& [i, health] : ZipperIndex<Health>(reg)) {
    if (!reg.is_entity_dying(i)) {
      health.damage_delta += dt;
      health.heal_delta += dt;
      this->_registry.get().emit<ComponentBuilder>(
          i,
          this->_registry.get().get_component_key<Health>(),
          health.to_bytes());
    }
  }
}

extern "C"
{
void* entry_point(Registry& r, EntityLoader& e)
{
  return new Life(r, e);
}
}
