#pragma once

#include <cmath>

#include "IPatternStrategy.hpp"

class CirclePattern : public IPatternStrategy
{
  static constexpr double DEFAULT_RADIUS = 0.2;

public:
  Vector2D calculate_position(Registry& registry,
                              Vector2D const& origin,
                              int index,
                              int total_count,
                              JsonObject const& params) const override
  {
    if (total_count <= 0) {
      return origin;
    }

    double radius = get_value_copy<double>(registry, params, "radius")
                        .value_or(DEFAULT_RADIUS);

    double angle = (TWO_PI * static_cast<double>(index))
        / static_cast<double>(total_count);
    return Vector2D(origin.x + radius * std::cos(angle),
                    origin.y + radius * std::sin(angle));
  }

  float calculate_direction_angle(Registry&  /*registry*/,
                                  int index,
                                  int total_count,
                                  JsonObject const&  /*params*/) const override
  {
    if (total_count <= 0) {
      return 0.0F;
    }
    return static_cast<float>((TWO_PI * static_cast<double>(index))
                              / static_cast<double>(total_count));
  }
};
