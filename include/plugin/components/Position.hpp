#pragma once

#include "ByteParser/ByteParser.hpp"
#include "plugin/Byte.hpp"
struct Position
{
    Position() = default;
    Position(double x, double y): x(x), y(y) {}

    DEFAULT_BYTE_CONSTRUCTOR(Position,
                             ([](double x, double y)
                              { return (Position){x, y}; }),
                             parseByte<double>(),
                             parseByte<double>())
    DEFAULT_SERIALIZE(type_to_byte(this->x), type_to_byte(this->y))

  double x;
  double y;
};
