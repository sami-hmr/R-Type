#pragma once

#include "ByteParser/ByteParser.hpp"
#include "ParserUtils.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"

struct Speed
{
  Speed() = default;

  Speed(double speed_x, double speed_y)
      : speed(speed_x, speed_y)
  {
  }

  Speed(Vector2D speed)
      : speed(speed)
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(Speed,
                           ([](double speed_x, double speed_y)
                            { return Speed {speed_x, speed_y}; }),
                           parseByte<double>(),
                           parseByte<double>())
  DEFAULT_SERIALIZE(type_to_byte(this->speed.x), type_to_byte(this->speed.y))

  CHANGE_ENTITY_DEFAULT

  Vector2D speed;

  HOOKABLE(Speed, HOOK(speed), HOOK(speed.x), HOOK(speed.y), )
};
