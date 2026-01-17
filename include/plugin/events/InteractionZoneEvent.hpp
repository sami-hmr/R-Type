#pragma once

#include "ByteParser/ByteParser.hpp"
#include "EventMacros.hpp"
#include "TwoWayMap.hpp"
#include "ecs/Registry.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"

struct InteractionZoneEvent
{
  Ecs::Entity source;
  double radius;
  std::vector<Ecs::Entity> candidates;

  CHANGE_ENTITY(result.source = map.at(source);
                result.candidates = MAP_ENTITY_VECTOR(candidates);)

  InteractionZoneEvent(Ecs::Entity src,
                       double r,
                       std::vector<Ecs::Entity> cands)
      : source(src)
      , radius(r)
      , candidates(std::move(cands))
  {
  }

  InteractionZoneEvent(Registry& r,
                       JsonObject const& e,
                       std::optional<Ecs::Entity> entity)
      : source(static_cast<Ecs::Entity>(
            get_value_copy<double>(r, e, "source", entity).value()))
      , radius(get_value_copy<double>(r, e, "radius", entity).value())
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(
      InteractionZoneEvent,
      ([](Ecs::Entity s, double r, std::vector<Ecs::Entity> const& c)
       { return InteractionZoneEvent(s, r, c); }),
      parseByte<Ecs::Entity>(),
      parseByte<double>(),
      parseByteArray<Ecs::Entity>(parseByte<Ecs::Entity>()))

  DEFAULT_SERIALIZE(type_to_byte(source),
                    type_to_byte(radius),
                    vector_to_byte(candidates,
                                   std::function<ByteArray(Ecs::Entity const&)>(
                                       [](Ecs::Entity const& e) {
                                         return type_to_byte<Ecs::Entity>(e);
                                       })))
};
