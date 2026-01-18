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

  Position(Vector2D pos, Vector2D offset = {0.0, 0.0}, int z = 1)
      : pos(pos)
      , offset(offset)
      , z(z)
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(
      Position,
      ([](Vector2D pos, Vector2D offset = {0.0, 0.0}, int z = 1)
       { return Position {pos, offset, z}; }),
      parseVector2D(),
      parseVector2D(),
      parseByte<int>())
  DEFAULT_SERIALIZE(vector2DToByte(this->pos),
                    vector2DToByte(this->offset),
                    type_to_byte(this->z))

  CHANGE_ENTITY_DEFAULT

  Vector2D pos;
  Vector2D offset;
  int z;
  bool applied_offset = false;
  HOOKABLE(Position, HOOK(pos), HOOK(offset), HOOK(z))
};
