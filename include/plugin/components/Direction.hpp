#pragma once

#include "ByteParser/ByteParser.hpp"
#include "ParserUtils.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"

struct Direction
{
  Direction() = default;

  Direction(double dir_x, double dir_y)
      : direction(dir_x, dir_y)
  {
  }

  Direction(Vector2D direction)
      : direction(direction)
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(Direction,
                           ([](double dir_x, double dir_y)
                            { return (Direction) {dir_x, dir_y}; }),
                           parseByte<double>(),
                           parseByte<double>())
  DEFAULT_SERIALIZE(type_to_byte(this->direction.x),
                    type_to_byte(this->direction.y))

  CHANGE_ENTITY_DEFAULT

  Vector2D direction;

  HOOKABLE(Direction, HOOK(direction), HOOK(direction.x), HOOK(direction.y), )
};
