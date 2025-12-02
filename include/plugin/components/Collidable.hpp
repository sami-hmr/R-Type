#pragma once

#include <string>
#include <vector>

enum class CollisionType
{
  Solid,
  Trigger
};

struct Collidable
{
  Collidable() = default;

  Collidable(double w,
             double h,
             CollisionType type = CollisionType::Solid,
             bool active = true)
      : width(w)
      , height(h)
      , collision_type(type)
      , is_active(active)
  {
  }

  Collidable(double w,
             double h,
             CollisionType type,
             bool active,
             const std::vector<std::string>& exclude)
      : width(w)
      , height(h)
      , collision_type(type)
      , is_active(active)
      , exclude_entities(exclude)
  {
  }

  double width = 0.0;
  double height = 0.0;
  CollisionType collision_type = CollisionType::Solid;
  bool is_active = true;
  std::vector<std::string> exclude_entities;
};
