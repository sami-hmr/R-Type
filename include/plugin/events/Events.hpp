#pragma once

#include <cstdint>
#include <map>
#include <optional>
#include <string>

#include "ParserUtils.hpp"
#include "ecs/Registery.hpp"
#include "plugin/Byte.hpp"

struct ShutdownEvent
{
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
  this->_registery.get().emit<LogEvent>(category, level, message);

enum class Key : std::uint8_t
{
  SHIFT,
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

struct CliComp
{
  CliComp() = default;
  EMPTY_BYTE_CONSTRUCTOR(CliComp)
  DEFAULT_SERIALIZE(ByteArray {})
};

struct CollisionEvent
{
  Registery::Entity a;
  Registery::Entity b;
};

struct HealEvent
{
  Registery::Entity target;
  Registery::Entity source;
  int amount;
};

struct DamageEvent
{
  Registery::Entity target;
  Registery::Entity source;
  int amount;
};
