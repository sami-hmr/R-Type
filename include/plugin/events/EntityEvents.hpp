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

  CollisionEvent(Registry::Entity a, Registry::Entity b)
      : a(a)
      , b(b)
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(CollisionEvent,
                           ([](Registry::Entity a, Registry::Entity b)
                            { return CollisionEvent(a, b); }),
                           parseByte<Registry::Entity>(),
                           parseByte<Registry::Entity>())

  DEFAULT_SERIALIZE(type_to_byte(this->a), type_to_byte(this->b))

  DEFINE_CHANGE_ENTITY(result.a = map.at_second(a);
                       result.b = map.at_second(b);)
};

struct InteractionZoneEvent
{
  Registry::Entity source;
  double radius;
  std::vector<Registry::Entity> candidates;

  InteractionZoneEvent(Registry::Entity s,
                       double r,
                       std::vector<Registry::Entity> c)
      : source(s)
      , radius(r)
      , candidates(std::move(c))
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(
      InteractionZoneEvent,
      ([](Registry::Entity s, double r, std::vector<Registry::Entity> const& c)
       { return InteractionZoneEvent(s, r, c); }),
      parseByte<Registry::Entity>(),
      parseByte<double>(),
      parseByteArray(parseByte<Registry::Entity>()))

  DEFINE_CHANGE_ENTITY(result.source = map.at_second(source);
                       result.candidates = MAP_ENTITY_VECTOR(candidates);)
};

struct HealEvent
{
  Registry::Entity target;
  Registry::Entity source;
  int amount;

  HealEvent(Registry::Entity target, Registry::Entity source, int amount)
      : target(target)
      , source(source)
      , amount(amount)
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(HealEvent,
                           ([](Registry::Entity t, Registry::Entity s, int a)
                            { return HealEvent(t, s, a); }),
                           parseByte<Registry::Entity>(),
                           parseByte<Registry::Entity>(),
                           parseByte<int>())

  DEFAULT_SERIALIZE(type_to_byte(this->target),
                    type_to_byte(this->source),
                    type_to_byte(this->amount))
  DEFINE_CHANGE_ENTITY(result.target = map.at_second(target);
                       result.source = map.at_second(source);)
};

struct DamageEvent
{
  Registry::Entity target;
  Registry::Entity source;
  int amount;

  DamageEvent(Registry::Entity target, Registry::Entity source, int amount)
      : target(target)
      , source(source)
      , amount(amount)
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(DamageEvent,
                           ([](Registry::Entity t, Registry::Entity s, int a)
                            { return DamageEvent(t, s, a); }),
                           parseByte<Registry::Entity>(),
                           parseByte<Registry::Entity>(),
                           parseByte<int>())

  DEFAULT_SERIALIZE(type_to_byte(this->target),
                    type_to_byte(this->source),
                    type_to_byte(this->amount))

  DEFINE_CHANGE_ENTITY(result.target = map.at_second(target);
                       result.source = map.at_second(source);)
};
