#pragma once

#include <cmath>

#include "IPatternStrategy.hpp"

class FormationVPattern : public IPatternStrategy
{
  static constexpr double DEFAULT_SPACING_X = 0.1;
  static constexpr double DEFAULT_SPACING_Y = 0.05;

public:
  Vector2D calculate_position(Registry& registry,
                              Vector2D const& origin,
                              int index,
                              int total_count,
                              JsonObject const& params) const override
  {
    double spacing_x = get_value_copy<double>(registry, params, "spacing_x")
                           .value_or(DEFAULT_SPACING_X);
    double spacing_y = get_value_copy<double>(registry, params, "spacing_y")
                           .value_or(DEFAULT_SPACING_Y);

    int center = total_count / 2;
    int offset = index - center;

    double offset_x = static_cast<double>(std::abs(offset)) * spacing_x;
    double offset_y = static_cast<double>(offset) * spacing_y;

    return Vector2D(origin.x + offset_x, origin.y + offset_y);
  }
};
