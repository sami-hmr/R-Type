#pragma once

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "ByteParser/ByteParser.hpp"
#include "EventMacros.hpp"
#include "Json/JsonParser.hpp"
#include "ParserUtils.hpp"
#include "TwoWayMap.hpp"
#include "ecs/Registry.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/components/ActionTrigger.hpp"
#include "plugin/components/InteractionZone.hpp"

enum Key : int
{
  Unknown = -1,
  SHIFT = 0,
  CTRL,
  ALT,
  ENTER,
  LEFT,
  RIGHT,
  DOWN,
  UP,
  Z,
  Q,
  S,
  D,
  R,
  ECHAP,
  DELETE,
  SPACE,
};

static const TwoWayMap<std::string, Key> KEY_MAPPING = {
    {"ENTER", Key::ENTER},
    {"R", Key::R},
    {"Z", Key::Z},
    {"Q", Key::Q},
    {"S", Key::S},
    {"D", Key::D},
    {"SPACE", Key::SPACE},
    {"ECHAP", Key::ECHAP},
    {"DELETE", Key::DELETE}};

struct KeyPressedEvent
{
  std::unordered_map<Key, bool> key_pressed;
  std::optional<std::string> key_unicode;

  CHANGE_ENTITY_DEFAULT

  KeyPressedEvent() = default;

  KeyPressedEvent(std::unordered_map<Key, bool> kp,
                  std::optional<std::string> ku)
      : key_pressed(std::move(kp))
      , key_unicode(std::move(ku))
  {
  }

  KeyPressedEvent(Registry& r, JsonObject const& e)
      : key_pressed(
            [&]() -> std::unordered_map<Key, bool>
            {
              JsonArray arr = get_value_copy<JsonArray>(r, e, "keys").value();
              std::unordered_map<Key, bool> map;
              for (auto const& i : arr) {
                map.insert_or_assign(
                    KEY_MAPPING.at_first(std::get<std::string>(i.value)), true);
              }
              return map;
            }())
      , key_unicode(get_value_copy<std::string>(r, e, "key_unicode"))
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(KeyPressedEvent,
                           ([](std::unordered_map<Key, bool> const& kp,
                               std::optional<std::string> const& ku)
                            { return KeyPressedEvent(kp, ku); }),
                           parseByteMap<Key, bool>(parseByte<Key>(),
                                                   parseByte<bool>()),
                           parseByteOptional(parseByteString()))

  DEFAULT_SERIALIZE(
      map_to_byte<Key, bool>(
          this->key_pressed,
          std::function<ByteArray(Key)>([](Key k) { return type_to_byte(k); }),
          std::function<ByteArray(bool)>([](bool b)
                                         { return type_to_byte(b); })),
      optional_to_byte<std::string>(this->key_unicode, string_to_byte))
};

struct KeyReleasedEvent
{
  std::unordered_map<Key, bool> key_released;
  std::optional<std::string> key_unicode;

  CHANGE_ENTITY_DEFAULT

  KeyReleasedEvent() = default;

  KeyReleasedEvent(std::unordered_map<Key, bool> kr, std::optional<std::string> ku)
      : key_released(std::move(kr))
      , key_unicode(std::move(ku))
  {
  }

  KeyReleasedEvent(Registry& r, JsonObject const& e)
      : key_released(
            [&]() -> std::unordered_map<Key, bool>
            {
              JsonArray arr = get_value_copy<JsonArray>(r, e, "keys").value();
              std::unordered_map<Key, bool> map;
              for (auto const& i : arr) {
                map.insert_or_assign(
                    KEY_MAPPING.at_first(std::get<std::string>(i.value)), true);
              }
              return map;
            }())
      , key_unicode(get_value_copy<std::string>(r, e, "key_unicode"))
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(KeyReleasedEvent,
                           ([](std::unordered_map<Key, bool> const& kp,
                               std::optional<std::string> const& ku)
                            { return KeyReleasedEvent(kp, ku); }),
                           parseByteMap<Key, bool>(parseByte<Key>(),
                                                   parseByte<bool>()),
                           parseByteOptional(parseByteString()))

  DEFAULT_SERIALIZE(
      map_to_byte<Key, bool>(
          this->key_released,
          std::function<ByteArray(Key)>([](Key k) { return type_to_byte(k); }),
          std::function<ByteArray(bool)>([](bool b)
                                         { return type_to_byte(b); })),
      optional_to_byte<std::string>(this->key_unicode, string_to_byte))
};

enum MouseButton : uint8_t
{
  MOUSELEFT = 0,
  MOUSERIGHT,
  MOUSEMIDDLE,
};

static const TwoWayMap<std::string, MouseButton> MOUSE_BUTTON_MAPPING = {
    {"MOUSELEFT", MouseButton::MOUSELEFT},
    {"MOUSERIGHT", MouseButton::MOUSERIGHT},
    {"MOUSEMIDDLE", MouseButton::MOUSEMIDDLE}};

struct MousePressedEvent
{
  Vector2D position;
  MouseButton button;

  CHANGE_ENTITY_DEFAULT

  MousePressedEvent() = default;
  MousePressedEvent(Vector2D pos, MouseButton btn)
      : position(pos)
      , button(btn)
  {
  }
  MousePressedEvent(Registry& r, JsonObject const& e)
      : position(get_value_copy<Vector2D>(r, e, "position").value())
      , button(static_cast<MouseButton>(
            get_value_copy<uint8_t>(r, e, "button").value()))
  {
  }
  DEFAULT_BYTE_CONSTRUCTOR(MousePressedEvent,
                           ([](Vector2D const& pos, MouseButton btn)
                            { return MousePressedEvent(pos, btn); }),
                           parseVector2D(),
                           parseByte<MouseButton>())
  DEFAULT_SERIALIZE(
       vector2DToByte(this->position), type_to_byte(this->button))
};

struct MouseReleasedEvent
{
  Vector2D position;
  MouseButton button;

  CHANGE_ENTITY_DEFAULT

  MouseReleasedEvent() = default;
  MouseReleasedEvent(Vector2D pos, MouseButton btn)
      : position(pos)
      , button(btn)
  {
  }
  MouseReleasedEvent(Registry& r, JsonObject const& e)
      : position(get_value_copy<Vector2D>(r, e, "position").value())
      , button(static_cast<MouseButton>(
            get_value_copy<uint8_t>(r, e, "button").value()))
  {
  }
  DEFAULT_BYTE_CONSTRUCTOR(MouseReleasedEvent,
                           ([](Vector2D const& pos, MouseButton btn)
                            { return MouseReleasedEvent(pos, btn); }),
                           parseVector2D(),
                           parseByte<MouseButton>())
  DEFAULT_SERIALIZE(
       vector2DToByte(this->position), type_to_byte(this->button))
};
