#pragma once

#include "IPatternStrategy.hpp"

class PointPattern : public IPatternStrategy
{
public:
  Vector2D calculate_position(Registry&  /*registry*/,
                              Vector2D const& origin,
                              int  /*index*/,
                              int  /*total_count*/,
                              JsonObject const&  /*params*/) const override
  {
    return origin;
  }
};
