#pragma once

#include "plugin/Hooks.hpp"
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

  HOOKABLE(Controllable, HOOK(up), HOOK(down), HOOK(left), HOOK(right))
};
