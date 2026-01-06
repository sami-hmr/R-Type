#pragma once

#include <vector>

#include "ByteParser/ByteParser.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/Byte.hpp"
#include "plugin/HookMacros.hpp"
#include "plugin/events/EventMacros.hpp"

struct Formation
{
  Formation() = default;

  Formation(double str)
      : strength(str)
      , active(true)
  {
  }

  Formation(double str, bool act)
      : strength(str)
      , active(act)
  {
  }

  HOOKABLE(Formation, HOOK(strength), HOOK(active))

  CHANGE_ENTITY_DEFAULT

  DEFAULT_BYTE_CONSTRUCTOR(
      Formation,
      ([](double str, bool act)
       { return Formation {str, act}; }),
      parseByte<double>(),
      parseByte<bool>())

  DEFAULT_SERIALIZE(
      type_to_byte(strength),
      type_to_byte(active))

  double strength;
  bool active;
};
