#pragma once

#include "ByteParser/ByteParser.hpp"
#include "ParserUtils.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/events/EventMacros.hpp"

struct Temporal
{
  Temporal() = default;

  Temporal(double life, double elapsed)
      : lifetime(life)
      , elapsed(elapsed)
  {
  }

  Temporal(double life)
      : lifetime(life)
      , elapsed(0.0)
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(Temporal,
                           ([](double lifetime, double elapsed)
                            { return Temporal{lifetime, elapsed}; }),
                           parseByte<double>(),
                           parseByte<double>())
  DEFAULT_SERIALIZE(type_to_byte(this->lifetime), type_to_byte(this->elapsed))

  HOOKABLE(Temporal, HOOK(lifetime))

  CHANGE_ENTITY_DEFAULT

  double lifetime;
  double elapsed;
};
