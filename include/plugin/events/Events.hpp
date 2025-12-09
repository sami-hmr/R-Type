#pragma once

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <utility>

#include "ParserUtils.hpp"
#include "plugin/Byte.hpp"
#include "ecs/Registry.hpp"
#include "plugin/events/Log.hpp"
#include "ByteParser/ByteParser.hpp"

#define LOGGER(category, level, message) \
  this->_registry.get().emit<LogEvent>(category, level, message);

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

struct KeyPressedEvent
{
  // KeyPressedEvent(std::map<Key, bool> kp, std::optional<std::string> ku)
  //   : key_pressed(std::move(kp))
  //   , key_unicode(std::move(ku)) {}

  // DEFAULT_BYTE_CONSTRUCTOR(KeyPressedEvent,
  //                        ([](std::map<Key, bool> const &kp, std::optional<std::string> const &ku)
  //                         { return (KeyPressedEvent) {kp, ku}; }),
  //                        parseByte<std::optional<std::string>>(),
  //                        parseByte<std::map<Key, bool>>())

  // DEFAULT_SERIALIZE(type_to_byte(this->key_pressed), type_to_byte(this->key_unicode))

  std::map<Key, bool> key_pressed;
  std::optional<std::string> key_unicode;
};

struct KeyReleasedEvent
{
  // KeyReleasedEvent(std::map<Key, bool> kp, std::optional<std::string> ku)
  //   : key_pressed(std::move(kp))
  //   , key_unicode(std::move(ku)) {}

  // DEFAULT_BYTE_CONSTRUCTOR(KeyReleasedEvent,
  //                        ([](std::map<Key, bool> const &kp, std::optional<std::string> const &ku)
  //                         { return (KeyReleasedEvent) {kp, ku}; }),
  //                        parseByte<std::optional<std::string>>(),
  //                        parseByte<std::map<Key, bool>>())

  // DEFAULT_SERIALIZE(type_to_byte(this->key_pressed), type_to_byte(this->key_unicode))

  std::map<Key, bool> key_released;
  std::optional<std::string> key_unicode;
};

struct CliStart
{
};

struct CliStop
{
};

struct CliComp
{
  CliComp() = default;
  EMPTY_BYTE_CONSTRUCTOR(CliComp)
  DEFAULT_SERIALIZE(ByteArray {})
};
