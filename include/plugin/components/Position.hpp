#pragma once

#include "ByteParser/ByteParser.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/Byte.hpp"

struct Position
{
  Position() = default;

  Position(double x, double y)
      : pos(x, y)
  {
  }

  Position(Vector2D pos)
      : pos(pos)
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(Position,
                           ([](double x, double y)
                            { return (Position) {x, y}; }),
                           parseByte<double>(),
                           parseByte<double>())
  DEFAULT_SERIALIZE(type_to_byte(this->pos.x), type_to_byte(this->pos.y))

  Vector2D pos;
};
