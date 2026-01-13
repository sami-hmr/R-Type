#pragma once

#include "ByteParser/ByteParser.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/events/EventMacros.hpp"

struct Health
{
  Health() = default;

  Health(double c, double m, double h, double d)
      : current(c)
      , max(m)
      , heal_delta(h)
      , damage_delta(d)
  {
  }

  Health(double current, double max)
      : current(current)
      , max(max)
      , heal_delta(0.0)
      , damage_delta(0.0)
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(Health,
                           ([](double c, double max, double h_d, double d_d)
                            { return Health{c, max, h_d, d_d}; }),
                           parseByte<double>(),
                           parseByte<double>(),
                           parseByte<double>(),
                           parseByte<double>())
  DEFAULT_SERIALIZE(type_to_byte(this->current),
                    type_to_byte(this->max),
                    type_to_byte(this->heal_delta),
                    type_to_byte(this->damage_delta))

  CHANGE_ENTITY_DEFAULT

  double current;
  double max;
  double heal_delta;
  double damage_delta;

  HOOKABLE(
      Health, HOOK(current), HOOK(max), HOOK(heal_delta), HOOK(damage_delta))
};
