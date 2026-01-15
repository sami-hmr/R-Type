#pragma once

#include <cmath>

#include "IPatternStrategy.hpp"

class GridPattern : public IPatternStrategy
{
  static constexpr int DEFAULT_COLS = 3;
  static constexpr double DEFAULT_SPACING = 0.1;

public:
  Vector2D calculate_position(Registry& registry,
                              Vector2D const& origin,
                              int index,
                              int  /*total_count*/,
                              JsonObject const& params) const override
  {
    int cols =
        static_cast<int>(get_value_copy<double>(registry, params, "cols")
                             .value_or(static_cast<double>(DEFAULT_COLS)));
    double spacing_x = get_value_copy<double>(registry, params, "spacing_x")
                           .value_or(DEFAULT_SPACING);
    double spacing_y = get_value_copy<double>(registry, params, "spacing_y")
                           .value_or(DEFAULT_SPACING);

    if (cols <= 0) {
      cols = 1;
    }

    int row = index / cols;
    int col = index % cols;

    return Vector2D(origin.x + static_cast<double>(col) * spacing_x,
                    origin.y + static_cast<double>(row) * spacing_y);
  }
};
