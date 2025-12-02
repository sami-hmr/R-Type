#pragma once

struct Rect
{
  double x;
  double y;
  double width;
  double height;

  bool intersects(Rect const& other) const
  {
    return x + width >= other.x && other.x + other.width >= x
             && y + height >= other.y && other.y + other.height >= y;
  }

  bool contains(double px, double py) const
  {
    return px >= x && px < x + width && py >= y && py < y + height;
  }
};