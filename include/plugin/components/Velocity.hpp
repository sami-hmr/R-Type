#pragma once

#include "ByteParser/ByteParser.hpp"
#include "ParserUtils.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"

struct Velocity
{
  Velocity() = default;

  Velocity(double speed_x, double speed_y, double dir_x, double dir_y)
      : speed(speed_x, speed_y)
      , direction(dir_x, dir_y)
  {
  }

  Velocity(Vector2D speed, Vector2D direction)
      : speed(speed)
      , direction(direction)
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(
      Velocity,
      ([](double speed_x, double speed_y, double dir_x, double dir_y)
       { return (Velocity) {speed_x, speed_y, dir_x, dir_y}; }),
      parseByte<double>(),
      parseByte<double>(),
      parseByte<double>(),
      parseByte<double>())
  DEFAULT_SERIALIZE(type_to_byte(this->speed.x),
                    type_to_byte(this->speed.y),
                    type_to_byte(this->direction.x),
                    type_to_byte(this->direction.y))

  CHANGE_ENTITY_DEFAULT

  Vector2D speed;
  Vector2D direction;

  HOOKABLE(Velocity,
           HOOK(speed),
           HOOK(direction),
           HOOK(direction.x),
           HOOK(direction.y),
           HOOK(speed.x),
           HOOK(speed.y), )
};
