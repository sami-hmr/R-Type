#pragma once

#include "ByteParser/ByteParser.hpp"
#include "libs/Color.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/Byte.hpp"
#include "plugin/HookMacros.hpp"
#include "plugin/Hooks.hpp"

struct Slider
{
  Vector2D size;
  Color bar_color;
  Color circle_color;
  double min_value;
  double max_value;
  double current_value;
  double step;
  bool selected = false;
  bool vertical = false;

  Slider(Vector2D size,
         Color bar_color,
         Color circle_color,
         double min_value,
         double max_value,
         double current_value,
         double step,
         bool selected = false,
         bool vertical = false)
      : size(size)
      , bar_color(bar_color)
      , circle_color(circle_color)
      , min_value(min_value)
      , max_value(max_value)
      , current_value(current_value)
      , step(step)
      , selected(selected)
      , vertical(vertical)
  {
  }

  CHANGE_ENTITY_DEFAULT

  DEFAULT_BYTE_CONSTRUCTOR(
      Slider,
      [](Vector2D size,
         Color bar_color,
         Color circle_color,
         double min_value,
         double max_value,
         double current_value,
         double step,
         bool selected,
         bool vertical)
      {
        return Slider(size,
                      bar_color,
                      circle_color,
                      min_value,
                      max_value,
                      current_value,
                      step,
                      selected,
                      vertical);
      },
      parseVector2D(),
      parseColor(),
      parseColor(),
      parseByte<double>(),
      parseByte<double>(),
      parseByte<double>(),
      parseByte<double>(),
      parseByte<bool>(),
      parseByte<bool>());

  DEFAULT_SERIALIZE(type_to_byte(this->size),
                    type_to_byte(this->bar_color),
                    type_to_byte(this->circle_color),
                    type_to_byte(this->min_value),
                    type_to_byte(this->max_value),
                    type_to_byte(this->current_value),
                    type_to_byte(this->step),
                    type_to_byte(this->selected),
                    type_to_byte(this->vertical))

  HOOKABLE(Slider,
           HOOK(size),
           HOOK(bar_color),
           HOOK(circle_color),
           HOOK(current_value),
           HOOK(max_value),
           HOOK(min_value),
           HOOK(step),
           HOOK(selected),
           HOOK(vertical))
};
