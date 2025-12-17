#pragma once

#include "ByteParser/ByteParser.hpp"
#include "Json/JsonParser.hpp"
#include "ecs/Registry.hpp"
#include "plugin/Byte.hpp"
#include "plugin/HookMacros.hpp"
#include "plugin/events/EventMacros.hpp"

struct Button
{
  bool pressed = false;
  bool hovered = false;
  bool toggle = false;

  Button() = default;

  Button(bool pressed, bool hovered, bool toggle)
      : pressed(pressed)
      , hovered(hovered)
      , toggle(toggle)
  {
  }

  CHANGE_ENTITY_DEFAULT

  DEFAULT_BYTE_CONSTRUCTOR(
      Button,
      [](bool pressed, bool hovered, bool toggle)
      { return Button(pressed, hovered, toggle); },
      parseByte<bool>(),
      parseByte<bool>(),
      parseByte<bool>())

  DEFAULT_SERIALIZE(type_to_byte(this->pressed),
                    type_to_byte(this->hovered),
                    type_to_byte(this->toggle))

  HOOKABLE(Button, HOOK(pressed), HOOK(hovered), HOOK(toggle));
};

void init_button(Registry& r, Registry::Entity const& e, JsonObject const& obj);
