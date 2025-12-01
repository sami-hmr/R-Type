#pragma once

#include <string>
#include <utility>
#include "Vector2D.hpp"

struct Sprite
{
  Sprite(std::string texture_path, Vector2D scale)
      : texture_path(std::move(texture_path)), scale(scale)
  {
  }

  std::string texture_path;
  Vector2D scale;
};
