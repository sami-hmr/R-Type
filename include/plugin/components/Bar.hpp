#pragma once

#include <string>

#include "ByteParser/ByteParser.hpp"
#include "libs/Color.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/Byte.hpp"
#include "plugin/HookMacros.hpp"
#include "plugin/events/EventMacros.hpp"

struct Bar
{
  Vector2D size;
  double max_value;
  double current_value;
  Vector2D offset = {0, 0};
  Color color = WHITE;
  std::string texture_path = "";
  bool outline = false;

  Bar(Vector2D size,
      double max_value,
      double current_value,
      Vector2D offset = {0, 0},
      Color color = WHITE,
      std::string texture_path = "",
      bool outline = false)
      : size(size)
      , max_value(max_value)
      , current_value(current_value)
      , offset(offset)
      , color(color)
      , texture_path(std::move(texture_path))
      , outline(outline)
  {
  }

  CHANGE_ENTITY_DEFAULT

  DEFAULT_BYTE_CONSTRUCTOR(
      Bar,
      [](Vector2D size,
         double max_value,
         double current_value,
         Vector2D offset,
         Color color,
         std::string texture_path,
         bool outline)
      {
        return Bar(size,
                   max_value,
                   current_value,
                   offset,
                   color,
                   std::move(texture_path),
                   outline);
      },
      parseVector2D(),
      parseByte<double>(),
      parseByte<double>(),
      parseVector2D(),
      parseColor(),
      parseByteString(),
      parseByte<bool>())

  DEFAULT_SERIALIZE(vector2DToByte(this->size),
                    type_to_byte<double>(this->max_value),
                    type_to_byte<double>(this->current_value),
                    vector2DToByte(this->offset),
                    colorToByte(this->color),
                    string_to_byte(this->texture_path),
                    type_to_byte<bool>(this->outline))
  
  HOOKABLE(Bar,
           HOOK(size),
           HOOK(max_value),
           HOOK(current_value),
           HOOK(offset),
           HOOK(color),
           HOOK(texture_path),
           HOOK(outline))
};
