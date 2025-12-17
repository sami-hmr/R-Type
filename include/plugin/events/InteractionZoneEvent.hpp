#pragma once

#include "ByteParser/ByteParser.hpp"
#include "EventMacros.hpp"
#include "TwoWayMap.hpp"
#include "ecs/Registry.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"

struct InteractionZoneEvent
{
  Registry::Entity source;
  double radius;
  std::vector<Registry::Entity> candidates;

  CHANGE_ENTITY(result.source = map.at(source);
                result.candidates = MAP_ENTITY_VECTOR(candidates);)

  InteractionZoneEvent(Registry::Entity src,
                       double r,
                       std::vector<Registry::Entity> cands)
      : source(src)
      , radius(r)
      , candidates(std::move(cands))
  {
  }

  InteractionZoneEvent(Registry& r, JsonObject const& e)
      : source(static_cast<Registry::Entity>(
            get_value_copy<double>(r, e, "source").value()))
      , radius(get_value_copy<double>(r, e, "radius").value())
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(
      InteractionZoneEvent,
      ([](Registry::Entity s, double r, std::vector<Registry::Entity> const& c)
       { return InteractionZoneEvent(s, r, c); }),
      parseByte<Registry::Entity>(),
      parseByte<double>(),
      parseByteArray<Registry::Entity>(parseByte<Registry::Entity>()))

  DEFAULT_SERIALIZE(
      type_to_byte(source),
      type_to_byte(radius),
      vector_to_byte(candidates,
                     std::function<ByteArray(Registry::Entity const&)>(
                         [](Registry::Entity const& e)
                         { return type_to_byte<Registry::Entity>(e); })))
};
