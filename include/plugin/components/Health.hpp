#pragma once

struct Health
{
  Health() = default;
  
  Health(int current, int max)
      : current(current)
      , max(max)
      , heal_delta(0.0)
      , damage_delta(0.0)
  {
  }

  int current;
  int max;
  double heal_delta;
  double damage_delta;
};
