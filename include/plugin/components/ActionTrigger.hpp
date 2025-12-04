#pragma once

#include <chrono>
#include <map>
#include <string>
#include <vector>

struct JsonValue;
using JsonObject = std::map<std::string, JsonValue>;

enum class TriggerType : std::uint8_t
{
  CLICK,
  HOVER,
  COLLISION,
  KEY_PRESS,
  TIMER,
  CUSTOM
};

enum class ActionType : std::uint8_t
{
  EMIT_EVENT,
  DELAY,
  KILL_ENTITY,
  SPAWN_ENTITY,
  MODIFY_COMPONENT
};

struct Action
{
  ActionType type;
  std::string event_name;
  JsonObject params;
  double delay_seconds;

  Action(ActionType t = ActionType::EMIT_EVENT,
         std::string event = "",
         JsonObject p = {},
         double delay = 0.0)
      : type(t)
      , event_name(std::move(event))
      , params(std::move(p))
      , delay_seconds(delay)
  {
  }
};

struct ActionTrigger
{
  TriggerType trigger;
  std::vector<Action> actions;
  std::string condition;
  bool triggered;
  std::chrono::steady_clock::time_point last_trigger_time;

  ActionTrigger(TriggerType t = TriggerType::CLICK,
                std::vector<Action> a = {},
                std::string cond = "")
      : trigger(t)
      , actions(std::move(a))
      , condition(std::move(cond))
      , triggered(false)
      , last_trigger_time(std::chrono::steady_clock::now())
  {
  }
};
