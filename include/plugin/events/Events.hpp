#pragma once

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "ByteParser/ByteParser.hpp"
#include "ParserUtils.hpp"
#include "ecs/Registry.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/components/ActionTrigger.hpp"
#include "plugin/components/InteractionZone.hpp"

struct ShutdownEvent
{
  DEFAULT_BYTE_CONSTRUCTOR(ShutdownEvent,
                           ([](std::string const& r, int e)
                            { return (ShutdownEvent) {r, e}; }),
                           parseByteString(),
                           parseByte<int>())

  DEFAULT_SERIALIZE(string_to_byte(this->reason), type_to_byte(this->exit_code))
  std::string reason;
  int exit_code = 0;

  ShutdownEvent(std::string reason, int exit_code)
      : reason(std::move(reason))
      , exit_code(exit_code)
  {
  }

  ShutdownEvent(Registry& r, JsonObject const& e)
      : reason(get_value_copy<std::string>(r, e, "reason").value())
      , exit_code(get_value_copy<int>(r, e, "exit_code").value())
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

struct InteractionZoneEvent
{
  Registry::Entity source;
  double radius;
  std::vector<Registry::Entity> candidates;
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
  std::string state;
  std::string reason;

  SceneChangeEvent(Registry& r, JsonObject const& e)
      : target_scene(get_value_copy<std::string>(r, e, "target_scene").value())
      , state(get_value_copy<std::string>(r, e, "state").value())
      , reason(get_value_copy<std::string>(r, e, "reason").value())
  {
  }
};
