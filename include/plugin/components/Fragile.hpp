#pragma once

#include "ParserUtils.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/events/EventMacros.hpp"

struct Fragile
{
  Fragile() = default;

  Fragile(int hits, int counter, double fragile_cooldown)
      : hits(hits)
      , counter(counter)
      , fragile_delta(fragile_cooldown)
  {
  }

  Fragile(int hits)
      : hits(hits)
      , counter(0)
      , fragile_delta(0.0)
  {
  }

  Fragile(int hits, double fragile_delta)
      : hits(hits)
      , counter(0)
      , fragile_delta(fragile_delta)
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(
      Fragile,
      ([](int hits, int counter, double fragile_delta)
       { return Fragile{hits, counter, fragile_delta}; }),
      parseByte<int>(),
      parseByte<int>(),
      parseByte<double>())
  DEFAULT_SERIALIZE(type_to_byte(this->hits),
                    type_to_byte(this->counter),
                    type_to_byte(this->fragile_delta))

  CHANGE_ENTITY_DEFAULT

  int hits;
  int counter;
  double fragile_delta;

  HOOKABLE(Fragile, HOOK(hits), HOOK(counter), HOOK(fragile_delta))
};
