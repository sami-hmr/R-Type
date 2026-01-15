#pragma once

#include "ByteParser/ByteParser.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"

struct Facing
{
  Facing() = default;

  Facing(double dir_x, double dir_y, bool plane = false)
      : direction(dir_x, dir_y), plane(plane)
  {
  }

  Facing(Vector2D direction , bool plane = false)
      : direction(direction), plane(plane)
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(Facing,
                           ([](double dir_x, double dir_y, bool plane)
                            { return (Facing) {dir_x, dir_y, plane}; }),
                           parseByte<double>(),
                           parseByte<double>(),
                           parseByte<bool>())
  DEFAULT_SERIALIZE(type_to_byte(this->direction.x),
                    type_to_byte(this->direction.y),
                    type_to_byte(this->plane))
  CHANGE_ENTITY_DEFAULT

  Vector2D direction;
  bool plane;

  HOOKABLE(Facing, HOOK(direction), HOOK(direction.x), HOOK(direction.y), HOOK(plane))
};
