#pragma once

#include "ByteParser/ByteParser.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"

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
  DEFAULT_SERIALIZE(type_to_byte(this->radius))

  double radius;
  bool enabled;

  HOOKABLE(InteractionZone, HOOK(radius), HOOK(enabled))
};
