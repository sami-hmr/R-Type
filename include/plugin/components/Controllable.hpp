#pragma once

struct Controllable
{
  Controllable() = default;

  Controllable(char up, char down, char left, char right)
      : up(up)
      , down(down)
      , left(left)
      , right(right)
  {
  }

  char up;
  char down;
  char left;
  char right;
};
