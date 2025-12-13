#pragma once
#include <functional>
#include <optional>
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

  DEFAULT_BYTE_CONSTRUCTOR(
      Text,
      (
          [](std::string font_path, double x, double y, std::string text)
          {
            return Text(std::move(font_path), Vector2D {x, y}, std::move(text));
          }),
      parseByteString(),
      parseByte<double>(),
      parseByte<double>(),
      parseByteString())

  DEFAULT_SERIALIZE(string_to_byte(this->font_path),
                    type_to_byte(this->scale.x),
                    type_to_byte(this->scale.y),
                    string_to_byte(this->text))

  CHANGE_ENTITY_DEFAULT

  std::string font_path;
  Vector2D scale;
  std::string text;

  HOOKABLE(Text, HOOK(font_path), HOOK(scale), HOOK(text))
};
