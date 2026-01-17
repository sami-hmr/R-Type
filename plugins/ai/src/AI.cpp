#include <iostream>

#include "AI.hpp"

#include "Json/JsonParser.hpp"
#include "attack/ContinuousFirePattern.hpp"
#include "ecs/zipper/ZipperIndex.hpp"
#include "movement/CirclePattern.hpp"
#include "movement/FollowTargetPattern.hpp"
#include "movement/GluePattern.hpp"
#include "movement/StraightPattern.hpp"
#include "movement/TurretPattern.hpp"
#include "movement/WavePattern.hpp"
#include "movement/ZigzagPattern.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/components/AttackBehavior.hpp"
#include "plugin/components/Direction.hpp"
#include "plugin/components/MovementBehavior.hpp"
#include "plugin/components/Position.hpp"
#include "plugin/components/Speed.hpp"

AI::AI(Registry& r, EventManager& em, EntityLoader& l)
    : APlugin("ai",
              r,
              em,
              l,
              {"moving", "collision", "target", "mob"},
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
  _movement_patterns["glue"] = std::make_unique<GluePattern>();

  _attack_patterns["continuous"] = std::make_unique<ContinuousFirePattern>();
  // _attack_patterns["aimed"] = std::make_unique<AimedFirePattern>();

  _registry.get().add_system(
      [this](Registry& r) { movement_behavior_system(r); }, 2);
  _registry.get().add_system([this](Registry& r) { attack_behavior_system(r); },
                             2);
}

void AI::init_movement_behavior(Ecs::Entity const& entity,
                                JsonObject const& obj)
{
  auto const& movement_type = get_value<MovementBehavior, std::string>(
      this->_registry.get(), obj, entity, "movement_type");

  if (!movement_type) {
    std::cerr << "Error loading MovementBehavior: missing movement_type\n";
    return;
  }

  JsonObject params;
  if (obj.contains("params")) {
    params = std::get<JsonObject>(obj.at("params").value);
  }

  this->_registry.get().emplace_component<MovementBehavior>(
      entity, movement_type.value(), params);
}

void AI::init_attack_behavior(Ecs::Entity const& entity,
                              JsonObject const& obj)
{
  auto const& attack_type = get_value<AttackBehavior, std::string>(
      this->_registry.get(), obj, entity, "attack_type");

  if (!attack_type) {
    std::cerr << "Error loading AttackBehavior: missing attack_type\n";
    return;
  }

  JsonObject params;
  if (obj.contains("params")) {
    params = std::get<JsonObject>(obj.at("params").value);
  }

  this->_registry.get().emplace_component<AttackBehavior>(
      entity, attack_type.value(), params);
}

void AI::movement_behavior_system(Registry& r)
{
  double dt = r.clock().delta_seconds();

  for (auto&& [entity, behavior, pos, speed, direction] :
       ZipperIndex<MovementBehavior, Position, Direction, Speed>(r))
  {
    if (_movement_patterns.find(behavior.movement_type)
        != _movement_patterns.end())
    {
      _movement_patterns[behavior.movement_type]->update(
          entity,
          r,
          this->_event_manager.get(),
          behavior,
          pos,
          speed,
          direction,
          dt);
    }
  }
}

void AI::attack_behavior_system(Registry& r)
{
  double dt = r.clock().delta_seconds();

  std::vector<std::function<void()>> to_exec;
  for (auto&& index :
       ZipperIndex<AttackBehavior, Position, Direction, Speed>(r))
  {
    std::size_t entity = std::get<0>(index);
    AttackBehavior const& behavior = std::get<1>(index);
    if (!behavior.active) {
      continue;
    }

    if (_attack_patterns.contains(behavior.attack_type)) {
      auto* pattern = _attack_patterns[behavior.attack_type].get();
      to_exec.emplace_back(
          [this, entity, pattern, dt]()
          {
            auto& behavior =
                *this->_registry.get().get_components<AttackBehavior>()[entity];
            auto& pos =
                *this->_registry.get().get_components<Position>()[entity];
            auto& speed =
                *this->_registry.get().get_components<Speed>()[entity];
            auto& direction =
                *this->_registry.get().get_components<Direction>()[entity];
            pattern->execute(entity,
                             this->_registry.get(),
                             this->_event_manager.get(),
                             behavior,
                             pos,
                             direction,
                             speed,
                             dt);
          });
    }
  }
  for (auto const& it : to_exec) {
    it();
  }
}

extern "C"
{
void* entry_point(Registry& r, EventManager& em, EntityLoader& e)
{
  return new AI(r, em, e);
}
}
