#pragma once

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "ByteParser/ByteParser.hpp"
#include "EventMacros.hpp"
#include "Json/JsonParser.hpp"
#include "ParserUtils.hpp"
#include "TwoWayMap.hpp"
#include "ecs/Registry.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/components/ActionTrigger.hpp"
#include "plugin/components/InteractionZone.hpp"

enum class Key
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
  std::map<Key, bool> key_pressed;
  std::optional<std::string> key_unicode;

  CHANGE_ENTITY_DEFAULT

  KeyPressedEvent(std::map<Key, bool> kp, std::optional<std::string> ku)
      : key_pressed(std::move(kp))
      , key_unicode(std::move(ku))
  {
  }

  KeyPressedEvent(Registry& r, JsonObject const& e)
      : key_pressed(
            [&]() -> std::map<Key, bool>
            {
              JsonArray arr = get_value_copy<JsonArray>(r, e, "keys").value();
              std::map<Key, bool> map;
              for (auto const& i : arr) {
                map.insert_or_assign(
                    KEY_MAPPING.at_first(std::get<std::string>(i.value)), true);
              }
              return map;
            }())
      , key_unicode(get_value_copy<std::string>(r, e, "key_unicode"))
  {
  }
};

struct KeyReleasedEvent
{
  std::map<Key, bool> key_released;
  std::optional<std::string> key_unicode;

  CHANGE_ENTITY_DEFAULT

  KeyReleasedEvent(std::map<Key, bool> kr, std::optional<std::string> ku)
      : key_released(std::move(kr))
      , key_unicode(std::move(ku))
  {
  }

  KeyReleasedEvent(Registry& r, JsonObject const& e)
      : key_released(
            [&]() -> std::map<Key, bool>
            {
              JsonArray arr = get_value_copy<JsonArray>(r, e, "keys").value();
              std::map<Key, bool> map;
              for (auto const& i : arr) {
                map.insert_or_assign(
                    KEY_MAPPING.at_first(std::get<std::string>(i.value)), true);
              }
              return map;
            }())
      , key_unicode(get_value_copy<std::string>(r, e, "key_unicode"))
  {
  }
};
