#pragma once

#include "ByteParser/ByteParser.hpp"
#include "ParserUtils.hpp"
#include "plugin/Byte.hpp"

struct Velocity
{
    Velocity() = default;
    Velocity(double x, double y): x(x), y(y) {}

    DEFAULT_BYTE_CONSTRUCTOR(Velocity,
                             ([](double x, double y)
                              { return (Velocity){x, y}; }),
                             parseByte<double>(),
                             parseByte<double>())
    DEFAULT_SERIALIZE(type_to_byte(this->x), type_to_byte(this->y))
  double x;
  double y;
};
