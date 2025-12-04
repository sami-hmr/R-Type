#pragma once

#include <cstdint>
#include <map>
#include <optional>
#include <string>

#include "ecs/Registery.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/components/ActionTrigger.hpp"

struct ShutdownEvent
{
  std::string reason;
  int exit_code = 0;

  ShutdownEvent(std::string reason, int exit_code)
      : reason(std::move(reason))
      , exit_code(exit_code)
  {
  }

  ShutdownEvent(Registery& r, JsonObject const& e)
      : reason(get_value<std::string>(r, e, "reason").value())
      , exit_code(get_value<int>(r, e, "exit_code").value())
  {
  }
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

struct SceneChangeEvent
{
  std::string target_scene;
  std::string reason;

  SceneChangeEvent(Registery& r, JsonObject const& e)
      : target_scene(get_value<std::string>(r, e, "target_scene").value())
      , reason(get_value<std::string>(r, e, "reason").value())
  {
  }
};
