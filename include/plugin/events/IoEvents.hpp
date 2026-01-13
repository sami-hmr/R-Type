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
  A,
  B,
  C,
  D,
  E,
  F,
  G,
  H,
  I,
  J,
  K,
  L,
  M,
  N,
  O,
  P,
  Q,
  R,
  S,
  T,
  U,
  V,
  W,
  X,
  Y,
  Z,
  ECHAP,
  DELETE,
  SPACE,
  SLASH,
  ONE,
  TWO,
  THREE,
  FOUR,
  FIVE,
  SIX,  // SIX SEVEN HAHAHAHA
  SEVEN,
  EIGHT,
  NINE,
  ZERO
};

static const TwoWayMap<std::string, Key> KEY_MAPPING = {
    {"ENTER", Key::ENTER},   {"SPACE", Key::SPACE}, {"ECHAP", Key::ECHAP},
    {"DELETE", Key::DELETE}, {"LEFT", Key::LEFT},   {"RIGHT", Key::RIGHT},
    {"UP", Key::UP},         {"DOWN", Key::DOWN},   {"SHIFT", Key::SHIFT},
    {"CTRL", Key::CTRL},     {"ALT", Key::ALT},     {"A", Key::A},
    {"B", Key::B},           {"C", Key::C},         {"D", Key::D},
    {"E", Key::E},           {"F", Key::F},         {"G", Key::G},
    {"H", Key::H},           {"I", Key::I},         {"J", Key::J},
    {"K", Key::K},           {"L", Key::L},         {"M", Key::M},
    {"N", Key::N},           {"O", Key::O},         {"P", Key::P},
    {"Q", Key::Q},           {"R", Key::R},         {"S", Key::S},
    {"T", Key::T},           {"U", Key::U},         {"V", Key::V},
    {"W", Key::W},           {"X", Key::X},         {"Y", Key::Y},
    {"Z", Key::Z},           {"/", Key::SLASH},     {"1", Key::ONE},
    {"2", Key::TWO},         {"3", Key::THREE},     {"4", Key::FOUR},
    {"5", Key::FIVE},        {"6", Key::SIX},       {"7", Key::SEVEN},
    {"8", Key::EIGHT},       {"9", Key::NINE},      {"0", Key::ZERO}};

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

  KeyReleasedEvent(std::unordered_map<Key, bool> kr,
                   std::optional<std::string> ku)
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
  DEFAULT_SERIALIZE(vector2DToByte(this->position), type_to_byte(this->button))
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
  DEFAULT_SERIALIZE(vector2DToByte(this->position), type_to_byte(this->button))
};

struct InputFocusEvent
{
  Registry::Entity entity;

  InputFocusEvent(Registry::Entity entity)
      : entity(entity)
  {
  }

  InputFocusEvent(Registry& r, JsonObject const& obj)
      : entity(static_cast<Registry::Entity>(
            get_value_copy<int>(r, obj, "entity").value())) {};

  CHANGE_ENTITY(result.entity = map.at(entity);)

  DEFAULT_BYTE_CONSTRUCTOR(InputFocusEvent,
                           ([](Registry::Entity entity)
                            { return InputFocusEvent(entity); }),
                           parseByte<int>())
  DEFAULT_SERIALIZE(type_to_byte(static_cast<int>(this->entity)))
};
