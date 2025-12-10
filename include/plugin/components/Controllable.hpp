#pragma once

#include "ByteParser/ByteParser.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/events/EventMacros.hpp"

struct Controllable
{
  Controllable() = default;

  Controllable(char up, char down, char left, char right)
      : up(up)
      , down(down)
      , left(left)
      , right(right)
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(Controllable,
                           ([](char u, char d, char l, char r)
                            { return Controllable(u, d, l, r); }),
                           parseByte<char>(),
                           parseByte<char>(),
                           parseByte<char>(),
                           parseByte<char>())

  DEFAULT_SERIALIZE(type_to_byte(up),
                    type_to_byte(down),
                    type_to_byte(left),
                    type_to_byte(right))

  CHANGE_ENTITY_DEFAULT

  char up;
  char down;
  char left;
  char right;

  HOOKABLE(Controllable, HOOK(up), HOOK(down), HOOK(left), HOOK(right))
};
