#include <iostream>

#include "AI.hpp"

#include "attack/AttackPatterns.hpp"
#include "ecs/zipper/ZipperIndex.hpp"
#include "movement/MovementPatterns.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/components/AttackBehavior.hpp"
#include "plugin/components/MovementBehavior.hpp"
#include "plugin/components/Position.hpp"
#include "plugin/components/Velocity.hpp"

AI::AI(Registry& r, EntityLoader& l)
    : APlugin("ai",
              r,
              l,
              {"moving", "collision", "target"},
              {COMP_INIT(
                   MovementBehavior, MovementBehavior, init_movement_behavior),
               COMP_INIT(AttackBehavior, AttackBehavior, init_attack_behavior)})
{
  REGISTER_COMPONENT(MovementBehavior)
  REGISTER_COMPONENT(AttackBehavior)

  _movement_patterns["straight"] = std::make_unique<StraightPattern>();
  _movement_patterns["wave"] = std::make_unique<WavePattern>();
  _movement_patterns["zigzag"] = std::make_unique<ZigzagPattern>();
  _movement_patterns["circle"] = std::make_unique<CirclePattern>();
  _movement_patterns["turret"] = std::make_unique<TurretPattern>();
  _movement_patterns["follow_target"] = std::make_unique<FollowTargetPattern>();

  _attack_patterns["continuous"] = std::make_unique<ContinuousFirePattern>();
  // _attack_patterns["aimed"] = std::make_unique<AimedFirePattern>();

  _registry.get().add_system(
      [this](Registry& r) { movement_behavior_system(r); }, 2);
  _registry.get().add_system([this](Registry& r) { attack_behavior_system(r); },
                             2);
}

void AI::init_movement_behavior(Registry::Entity const& entity,
                                JsonObject const& obj)
{
  auto const& movement_type = get_value<MovementBehavior, std::string>(
      this->_registry.get(), obj, entity, "movement_type");

  if (!movement_type) {
    std::cerr << "Error loading MovementBehavior: missing movement_type\n";
    return;
  }

  this->_registry.get().emplace_component<MovementBehavior>(
      entity, movement_type.value());
}

void AI::init_attack_behavior(Registry::Entity const& entity,
                              JsonObject const& obj)
{
  auto const& attack_type = get_value<AttackBehavior, std::string>(
      this->_registry.get(), obj, entity, "attack_type");
  auto const& attack_interval = get_value<AttackBehavior, double>(
      this->_registry.get(), obj, entity, "attack_interval");

  if (!attack_type || !attack_interval) {
    std::cerr << "Error loading AttackBehavior: missing attack_type or "
                 "attack_interval\n";
    return;
  }

  this->_registry.get().emplace_component<AttackBehavior>(
      entity, attack_type.value(), attack_interval.value());
}

void AI::movement_behavior_system(Registry& r)
{
  double dt = r.clock().delta_seconds();

  for (auto&& [entity, behavior, pos, vel] :
       ZipperIndex<MovementBehavior, Position, Velocity>(r))
  {
    if (_movement_patterns.find(behavior.movement_type)
        != _movement_patterns.end())
    {
      _movement_patterns[behavior.movement_type]->update(
          entity, r, behavior, pos, vel, dt);
    }
  }
}

void AI::attack_behavior_system(Registry& r)
{
  double dt = r.clock().delta_seconds();

  for (auto&& [entity, behavior, pos, vel] :
       ZipperIndex<AttackBehavior, Position, Velocity>(r))
  {
    if (!behavior.active) {
      continue;
    }

    if (_attack_patterns.find(behavior.attack_type) != _attack_patterns.end()) {
      _attack_patterns[behavior.attack_type]->execute(
          entity, r, behavior, pos, vel, dt);
    }
  }
}

extern "C"
{
void* entry_point(Registry& r, EntityLoader& e)
{
  return new AI(r, e);
}
}
