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
                           ([](Vector2D pos, int z = 1)
                            { return Position {pos, z}; }),
                           parseVector2D(),
                           parseByte<int>())
  DEFAULT_SERIALIZE(vector2DToByte(this->pos), type_to_byte(this->z))

  CHANGE_ENTITY_DEFAULT

  Vector2D pos;
  int z;
  bool applied_offset = false;
  HOOKABLE(Position, HOOK(pos), HOOK(pos.x), HOOK(pos.y), HOOK(z))
};

struct Offset
{
  Offset() = default;

  Offset(double x, double y)
      : offset(x, y)
  {
  }

  Offset(Vector2D offset)
      : offset(offset)
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(Offset,
                           ([](Vector2D offset) { return Offset {offset}; }),
                           parseVector2D())

  DEFAULT_SERIALIZE(vector2DToByte(this->offset))

  CHANGE_ENTITY_DEFAULT

  Vector2D offset;
  HOOKABLE(Offset, HOOK(offset))
};
