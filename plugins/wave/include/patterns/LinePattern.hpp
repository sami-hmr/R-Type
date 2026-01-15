#pragma once

#include "IPatternStrategy.hpp"

class LinePattern : public IPatternStrategy
{
  static constexpr double DEFAULT_SPACING = 0.1;

public:
  Vector2D calculate_position(Registry& registry,
                              Vector2D const& origin,
                              int index,
                              int  /*total_count*/,
                              JsonObject const& params) const override
  {
    double spacing = get_value_copy<double>(registry, params, "spacing")
                         .value_or(DEFAULT_SPACING);
    return Vector2D(origin.x + static_cast<double>(index) * spacing, origin.y);
  }
};
