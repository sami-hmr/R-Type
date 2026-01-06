#pragma once

#include "Json/JsonParser.hpp"
#include "ecs/Registry.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/components/MovementBehavior.hpp"
#include "plugin/components/AttackBehavior.hpp"
#include "plugin/components/Position.hpp"
#include "plugin/components/Direction.hpp"
#include "plugin/components/Speed.hpp"
#include "movement/MovementPattern.hpp"
#include "attack/AttackPattern.hpp"
#include <memory>
#include <unordered_map>

class AI : public APlugin
{
public:
  AI(Registry& r, EventManager &em, EntityLoader& l);

private:
  void init_movement_behavior(Registry::Entity const& entity, JsonObject const& obj);
  void init_attack_behavior(Registry::Entity const& entity, JsonObject const& obj);

  void movement_behavior_system(Registry& r);
  void attack_behavior_system(Registry& r);

  std::unordered_map<std::string, std::unique_ptr<MovementPattern>> _movement_patterns;
  std::unordered_map<std::string, std::unique_ptr<AttackPattern>> _attack_patterns;
};
