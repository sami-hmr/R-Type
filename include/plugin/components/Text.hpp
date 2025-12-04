#pragma once
#include <string>
#include <vector>

#include "BaseTypes.hpp"
#include "ByteParser/ByteParser.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"

struct Text
{
  Text(std::string font_path, Vector2D v, std::string t)
      : font_path(std::move(font_path))
      , scale(v)
      , text(std::move(t))
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(Text,
                           (
                               [](std::vector<char> font_path,
                                  double x,
                                  double y,
                                  std::vector<char> text)
                               {
                                 return Text(
                                     std::string(font_path.begin(),
                                                 font_path.end()),
                                     Vector2D{x, y},
                                     std::string(text.begin(), text.end()));
                               }),
                           parseByteArray(parseAnyChar()), parseByte<double>(), parseByte<double>(), parseByteArray(parseAnyChar()))
  DEFAULT_SERIALIZE(string_to_byte(this->font_path), type_to_byte(this->scale.x), type_to_byte(this->scale.y), string_to_byte(this->text))

  std::string font_path;
  Vector2D scale;
  std::string text;

  HOOKABLE(Text, HOOK(font_path), HOOK(scale), HOOK(text))

};
