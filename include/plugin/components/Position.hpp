#pragma once

struct Position
{
  Position() = default;
  
  Position(double x, double y)
      : x(x)
      , y(y)
  {
  }

  double x;
  double y;
};
