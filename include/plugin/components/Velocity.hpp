#pragma once

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

  double speed_x;
  double speed_y;
  double dir_x;
  double dir_y;
};
