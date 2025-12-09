#pragma once

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "ecs/Registery.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/components/ActionTrigger.hpp"
#include "plugin/components/InteractionZone.hpp"

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

struct InteractionZoneEvent
{
  Registery::Entity source;
  double radius;
  std::vector<Registery::Entity> candidates;
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
  std::string state;
  std::string reason;

  SceneChangeEvent(Registery& r, JsonObject const& e)
      : target_scene(get_value_copy<std::string>(r, e, "target_scene").value())
      , state(get_value_copy<std::string>(r, e, "state").value())
      , reason(get_value_copy<std::string>(r, e, "reason").value())
  {
  }
};

struct AnimationEndEvent
{
  std::string name;
  Registery::Entity entity;

  AnimationEndEvent(std::string name, Registery::Entity entity)
      : name(std::move(name))
      , entity(entity) {};

  AnimationEndEvent(Registery& r, JsonObject const& e)
      : name(get_value_copy<std::string>(r, e, "name").value())
      , entity(get_value_copy<int>(r, e, "entity").value())
  {
  }
};

struct AnimationStartEvent
{
  std::string name;
  Registery::Entity entity;

  AnimationStartEvent(std::string name, Registery::Entity entity)
      : name(std::move(name))
      , entity(entity) {};

  AnimationStartEvent(Registery& r, JsonObject const& e)
      : name(get_value_copy<std::string>(r, e, "name").value())
      , entity(get_value_copy<int>(r, e, "entity").value())
  {
  }
};

struct PlayAnimationEvent
{
  std::string name;
  Registery::Entity entity;
  double framerate;
  bool loop;
  bool rollback;

  PlayAnimationEvent(std::string name,
                     Registery::Entity entity,
                     double framerate,
                     bool loop,
                     bool rollback)
      : name(std::move(name))
      , entity(entity)
      , framerate(framerate)
      , loop(loop)
      , rollback(rollback)
  {
  }

  PlayAnimationEvent(Registery& r, JsonObject const& e)
      : name(get_value_copy<std::string>(r, e, "name").value())
      , entity(get_value_copy<int>(r, e, "entity").value())
      , framerate(get_value_copy<double>(r, e, "framerate").value())
      , loop(get_value_copy<bool>(r, e, "loop").value())
      , rollback(get_value_copy<bool>(r, e, "rollback").value())
  {
  }
};
