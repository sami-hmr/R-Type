#pragma once
#include <functional>
#include <optional>
#include <string>
#include <vector>

#include "BaseTypes.hpp"
#include "ByteParser/ByteParser.hpp"
#include "libs/Color.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"

struct Text
{
  Text(std::string font_path,
       Vector2D v,
       std::string t,
       std::string placeholder,
       Color outline_color,
       Color fill_color,
       bool outline,
       double outline_thickness)
      : font_path(std::move(font_path))
      , scale(v)
      , text(std::move(t))
      , placeholder(std::move(placeholder))
      , outline_color(outline_color)
      , fill_color(fill_color)
      , outline(outline)
      , outline_thickness(outline_thickness)
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(Text,
                           (
                               [](std::string font_path,
                                  double x,
                                  double y,
                                  std::string const &text,
                                  std::string const &placeholder,
                                  Color outline_color,
                                  Color fill_color,
                                  bool outline,
                                  double outline_thickness)
                               {
                                 return Text(std::move(font_path),
                                             Vector2D {x, y},
                                             text,
                                             placeholder,
                                             outline_color,
                                             fill_color,
                                             outline,
                                             outline_thickness);
                               }),
                           parseByteString(),
                           parseByte<double>(),
                           parseByte<double>(),
                           parseByteString(),
                           parseByteString(),
                           parseColor(),
                           parseColor(),
                           parseByte<bool>(),
                           parseByte<double>())

  DEFAULT_SERIALIZE(string_to_byte(this->font_path),
                    type_to_byte(this->scale.x),
                    type_to_byte(this->scale.y),
                    string_to_byte(this->text),
                    string_to_byte(this->placeholder),
                    colorToByte(this->outline_color),
                    colorToByte(this->fill_color),
                    type_to_byte<bool>(this->outline),
                    type_to_byte<double>(this->outline_thickness))

  CHANGE_ENTITY_DEFAULT

  std::string font_path;
  Vector2D scale;
  std::string text;
  std::string placeholder;
  Color outline_color = BLACK;
  Color fill_color = WHITE;
  bool outline = false;
  double outline_thickness = 1.0f;

  HOOKABLE(Text,
           HOOK(font_path),
           HOOK(scale),
           HOOK(text),
           HOOK(placeholder),
           HOOK(outline_color),
           HOOK(fill_color),
           HOOK(outline),
           HOOK(outline_thickness))
};
