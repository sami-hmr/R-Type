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
  double width = 0.0;
  double height = 0.0;
  CollisionType collision_type = CollisionType::Solid;
  bool is_active = true;
  std::vector<std::string> exclude_entities;
};
