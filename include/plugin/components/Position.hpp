#pragma once

#include "ByteParser/ByteParser.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"

struct Position
{
  Position() = default;

  Position(double x, double y, int z = 1)
      : pos(x, y)
      , z(z)
  {
  }

  Position(Vector2D pos, int z = 1)
      : pos(pos)
      , z(z)
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(Position,
                           ([](double x, double y, int z)
                            { return (Position) {x, y, z}; }),
                           parseByte<double>(),
                           parseByte<double>(),
                           parseByte<int>())
  DEFAULT_SERIALIZE(type_to_byte(this->pos.x),
                    type_to_byte(this->pos.y),
                    type_to_byte(this->z))
  Vector2D pos;
  int z;
  HOOKABLE(Position, HOOK(pos), HOOK(z))
};
