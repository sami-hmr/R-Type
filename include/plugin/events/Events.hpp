#pragma once

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <utility>

#include "ByteParser/ByteParser.hpp"
#include "ParserUtils.hpp"
#include "ecs/Registry.hpp"
#include "plugin/Byte.hpp"

struct ShutdownEvent
{
  ShutdownEvent(std::string r, int e)
      : reason(std::move(r))
      , exit_code(e)
  {
  }
  DEFAULT_BYTE_CONSTRUCTOR(ShutdownEvent,
                           ([](std::string const& r, int e)
                            { return (ShutdownEvent) {r, e}; }),
                           parseByteString(),
                           parseByte<int>())

  DEFAULT_SERIALIZE(string_to_byte(this->reason), type_to_byte(this->exit_code))
  std::string reason;
  int exit_code = 0;
};

struct CleanupEvent
{
  std::string trigger;
};

enum class LogLevel : std::uint8_t
{
  DEBUG,
  INFO,
  WARNING,
  ERROR
};

struct LogEvent
{
  std::string name;
  LogLevel level;
  std::string message;
};

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
  std::map<Key, bool> key_pressed;
  std::optional<std::string> key_unicode;
};

struct KeyReleasedEvent
{
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

struct CollisionEvent
{
  Registry::Entity a;
  Registry::Entity b;
};

struct HealEvent
{
  Registry::Entity target;
  Registry::Entity source;
  int amount;
};

struct DamageEvent
{
  Registry::Entity target;
  Registry::Entity source;
  int amount;
};

struct SceneChangeEvent
{
  std::string target_scene;
  std::string reason;
};
