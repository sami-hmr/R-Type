#pragma once

#include <string>

struct Sprite
{
  Sprite(std::string texture_path)
      : texture_path(std::move(texture_path))
  {
  }

  std::string texture_path;
};
