#pragma once

#include "ByteParser/ByteParser.hpp"
#include "Json/JsonParser.hpp"
#include "ecs/Registry.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/events/EventMacros.hpp"

struct InteractionZone
{
  InteractionZone() = default;

  InteractionZone(double radius)
      : radius(radius)
      , enabled(true)
  {
  }

  InteractionZone(double radius, bool enabled)
      : radius(radius)
      , enabled(enabled)
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(InteractionZone,
                           ([](double radius, bool enabled)
                            { return (InteractionZone) {radius, enabled}; }),
                           parseByte<double>(),
                           parseByte<bool>())
  DEFAULT_SERIALIZE(type_to_byte(this->radius), type_to_byte(this->enabled))

  CHANGE_ENTITY_DEFAULT

  double radius;
  bool enabled;

  InteractionZone(Registry& r, JsonObject const& o)
      : radius(get_value_copy<double>(r, o, "radius").value())
      , enabled(get_value_copy<bool>(r, o, "enable").value())
  {
  }

  HOOKABLE(InteractionZone, HOOK(radius), HOOK(enabled))
};
