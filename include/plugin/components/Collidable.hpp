#pragma once

#include <string>
#include <vector>

struct Collidable
{
  Collidable() = default;

  Collidable(double width, double height)
      : width(width)
      , height(height)
  {
  }

  Collidable(double width,
             double height,
             std::vector<std::string> const& exclude)
      : width(width)
      , height(height)
      , exclude_entities(exclude)
  {
  }

  double width = 0.0;
  double height = 0.0;
  std::vector<std::string> exclude_entities;
};
