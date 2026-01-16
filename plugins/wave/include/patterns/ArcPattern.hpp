#pragma once

#include <cmath>

#include "IPatternStrategy.hpp"

class ArcPattern : public IPatternStrategy
{
  static constexpr double DEFAULT_ARC_ANGLE = 45.0;

public:
  Vector2D calculate_position(Registry&  /*registry*/,
                              Vector2D const& origin,
                              int  /*index*/,
                              int  /*total_count*/,
                              JsonObject const&  /*params*/) const override
  {
    return origin;
  }

  float calculate_direction_angle(Registry& registry,
                                  int index,
                                  int total_count,
                                  JsonObject const& params) const override
  {
    if (total_count <= 1) {
      return 0.0F;
    }

    double arc_angle_deg = get_value_copy<double>(registry, params, "angle")
                               .value_or(DEFAULT_ARC_ANGLE);

    double arc_angle_rad = arc_angle_deg * DEG_TO_RAD;
    double half_arc = arc_angle_rad / 2.0;
    double step = arc_angle_rad / static_cast<double>(total_count - 1);

    return static_cast<float>(-half_arc + static_cast<double>(index) * step);
  }
};
