#pragma once
#include <string>
#include "Vector2D.hpp"

struct Text
{
  Text (std::string font_path, Vector2D scale, std::string text) : font_path(std::move(font_path)), scale(scale), text(std::move(text)) {}

  std::string font_path;
  Vector2D scale;
  std::string text;
};
