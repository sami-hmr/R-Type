#pragma once

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "ByteParser/ByteParser.hpp"
#include "EventMacros.hpp"
#include "ParserUtils.hpp"
#include "TwoWayMap.hpp"
#include "ecs/Registry.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/components/ActionTrigger.hpp"
#include "plugin/components/InteractionZone.hpp"

struct CollisionEvent
{
  Registry::Entity a;
  Registry::Entity b;

  DEFINE_CHANGE_ENTITY(result.a = map.at_second(a);
                       result.b = map.at_second(b);)
};

struct InteractionZoneEvent
{
  Registry::Entity source;
  double radius;
  std::vector<Registry::Entity> candidates;

  DEFINE_CHANGE_ENTITY(result.source = map.at_second(source);
                       result.candidates = MAP_ENTITY_VECTOR(candidates);)
};

struct HealEvent
{
  Registry::Entity target;
  Registry::Entity source;
  int amount;

  DEFINE_CHANGE_ENTITY(result.target = map.at_second(target);
                       result.source = map.at_second(source);)
};

struct DamageEvent
{
  Registry::Entity target;
  Registry::Entity source;
  int amount;

  DEFINE_CHANGE_ENTITY(result.target = map.at_second(target);
                       result.source = map.at_second(source);)
};
