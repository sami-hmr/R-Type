#pragma once
#include <string>

struct Text
{
  Text() = default;
  
  Text(std::string font_path)
      : font_path(std::move(font_path))
  {
  }

  std::string font_path;
};
