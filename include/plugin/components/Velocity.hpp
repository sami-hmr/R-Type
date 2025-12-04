#pragma once

#include "ByteParser/ByteParser.hpp"
#include "ParserUtils.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"

struct Velocity
{
  Velocity() = default;

  Velocity(double speed_x, double speed_y, double dir_x, double dir_y)
      : speed_x(speed_x)
      , speed_y(speed_y)
      , dir_x(dir_x)
      , dir_y(dir_y)
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
  DEFAULT_SERIALIZE(type_to_byte(this->speed_x),
                    type_to_byte(this->speed_y),
                    type_to_byte(this->dir_x),
                    type_to_byte(this->dir_y))

  double speed_x;
  double speed_y;
  double dir_x;
  double dir_y;

  HOOKABLE(Velocity, HOOK(speed_x), HOOK(speed_y), HOOK(dir_x), HOOK(dir_y))

};
