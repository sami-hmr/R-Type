#pragma once

#include <cmath>
#include <numbers>

#include "Json/JsonParser.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/Hooks.hpp"

class Registry;

class IPatternStrategy
{
public:
  static constexpr double PI = std::numbers::pi;
  static constexpr double TWO_PI = 2.0 * PI;
  static constexpr double DEG_TO_RAD = PI / 180.0;

  IPatternStrategy() = default;
  virtual ~IPatternStrategy() = default;

  IPatternStrategy(const IPatternStrategy&) = default;
  IPatternStrategy& operator=(const IPatternStrategy&) = default;
  IPatternStrategy(IPatternStrategy&&) = default;
  IPatternStrategy& operator=(IPatternStrategy&&) = default;

  virtual Vector2D calculate_position(Registry& registry,
                                      Vector2D const& origin,
                                      int index,
                                      int total_count,
                                      JsonObject const& params) const = 0;

  virtual float calculate_direction_angle(Registry&  /*registry*/,
                                          int  /*index*/,
                                          int  /*total_count*/,
                                          JsonObject const&  /*params*/) const
  {
    return 0.0F;
  }
};
