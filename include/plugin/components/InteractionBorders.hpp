#pragma once

#include <unordered_set>

#include "ByteParser/ByteParser.hpp"
#include "ecs/Registry.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"

struct InteractionBorders
{
  bool enabled;
  double radius;
  std::unordered_set<Ecs::Entity> in_zone;

  InteractionBorders(double r, std::unordered_set<Ecs::Entity> in_zone)
      : enabled(true)
      , radius(r)
      , in_zone(std::move(in_zone))
  {
  }

  InteractionBorders(bool enabled,
                     double r,
                     std::unordered_set<Ecs::Entity> in_zone)
      : enabled(enabled)
      , radius(r)
      , in_zone(std::move(in_zone))
  {
  }

  InteractionBorders(Registry& r, JsonObject const& e)
      : enabled(get_value_copy<bool>(r, e, "enabled").value())
      , radius(get_value_copy<double>(r, e, "radius").value())
      , in_zone(get_value_copy<std::unordered_set<Ecs::Entity>>(
                    r, e, "in_zone")
                    .value())
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(
      InteractionBorders,
      (
          [](bool e, double r, std::vector<Ecs::Entity> const& i) {
            return InteractionBorders(
                e, r, std::unordered_set(i.begin(), i.end()));
          }),
      parseByte<bool>(),
      parseByte<double>(),
      parseByteArray<Ecs::Entity>(parseByte<Ecs::Entity>()))

  DEFAULT_SERIALIZE(type_to_byte(enabled),
                    type_to_byte(radius),
                    [this]()
                    {
                      std::vector<Ecs::Entity> v(in_zone.begin(),
                                                      in_zone.end());
                      return vector_to_byte(
                          v,
                          std::function<ByteArray(Ecs::Entity const&)>(
                              [](Ecs::Entity const& e)
                              { return type_to_byte<Ecs::Entity>(e); }));
                    }())

  HOOKABLE(InteractionBorders, HOOK(radius), HOOK(enabled));

  CHANGE_ENTITY_DEFAULT
};
