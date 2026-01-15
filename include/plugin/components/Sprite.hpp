#pragma once

#include <string>
#include <utility>

#include "ByteParser/ByteParser.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"

struct Sprite
{
  Sprite(std::string texture_path, Vector2D scale)
      : texture_path(std::move(texture_path))
      , scale(scale)
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(
      Sprite,
      (
          [](std::vector<char> texture_path, double x, double y)
          {
            return Sprite(std::string(texture_path.begin(), texture_path.end()),
                          Vector2D {x, y});
          }),
      parseByteArray(parseAnyChar()),
      parseByte<double>(),
      parseByte<double>())
  DEFAULT_SERIALIZE(string_to_byte(this->texture_path),
                    type_to_byte(this->scale.x),
                    type_to_byte(this->scale.y))

  CHANGE_ENTITY_DEFAULT

  std::string texture_path;
  Vector2D scale;
  
  HOOKABLE(Sprite, HOOK(texture_path), HOOK(scale))
};
