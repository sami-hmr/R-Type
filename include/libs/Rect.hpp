#pragma once

#include <iostream>
struct Rect
{
  double x;
  double y;
  double width;
  double height;

  bool intersects(Rect const& other) const
  {
    double left = x - (width / 2);
    double right = x + (width / 2);
    double top = y - (height / 2);
    double bottom = y + (height / 2);

    double other_left = other.x - (other.width / 2);
    double other_right = other.x + (other.width / 2);
    double other_top = other.y - (other.height / 2);
    double other_bottom = other.y + (other.height / 2);

    return right >= other_left && other_right >= left && bottom >= other_top
        && other_bottom >= top;
  }

  bool contains(double px, double py) const
  {
    double left = x - (width / 2);
    double right = x + (width / 2);
    double top = y - (height / 2);
    double bottom = y + (height / 2);

    return px >= left && px < right && py >= top && py < bottom;
  }
};
