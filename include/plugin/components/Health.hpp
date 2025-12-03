#pragma once

#include "ByteParser/ByteParser.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"

struct Health
{
  Health() = default;

  Health(int c, int m, double h, double d)
      : current(c)
      , max(m)
      , heal_delta(h)
      , damage_delta(d)
  {
  }

  Health(int current, int max)
      : current(current)
      , max(max)
      , heal_delta(0.0)
      , damage_delta(0.0)
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(Health,
                           ([](int c, int max, double h_d, double d_d)
                            { return (Health) {c, max, h_d, d_d}; }),
                           parseByte<int>(),
                           parseByte<int>(),
                           parseByte<double>(),
                           parseByte<double>())
  DEFAULT_SERIALIZE(type_to_byte(this->current),
                    type_to_byte(this->max),
                    type_to_byte(this->heal_delta),
                    type_to_byte(this->damage_delta))

  int current;
  int max;
  double heal_delta;
  double damage_delta;

  HOOKABLE(Health, HOOK(Health, current), HOOK(Health, max), HOOK(Health, heal_delta), HOOK(Health, damage_delta))
};
