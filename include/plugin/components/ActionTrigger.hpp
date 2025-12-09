#pragma once

#include <chrono>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "ByteParser/ByteParser.hpp"
#include "Json/JsonParser.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"

struct ActionTrigger
{
  ActionTrigger()
      : triggered(false)
      , last_trigger_time(std::chrono::steady_clock::now())
  {
  }

  ActionTrigger(bool triggered)
      : triggered(triggered)
  {
  }

  ActionTrigger(std::pair<std::string, JsonObject> const& evt_trg,
                std::vector<std::pair<std::string, JsonObject>> const& evt_emt)
      : event_trigger(evt_trg)
      , event_to_emit(evt_emt)
      , triggered(false)
      , last_trigger_time(std::chrono::steady_clock::now())
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(ActionTrigger,
                           ([](bool triggered)
                            { return ActionTrigger(triggered); }),
                           parseByte<bool>())
  DEFAULT_SERIALIZE(string_to_byte(this->event_trigger.first),
                    string_to_byte(this->event_trigger.first))

  std::pair<std::string, JsonObject> event_trigger;
  std::vector<std::pair<std::string, JsonObject>> event_to_emit;
  bool triggered;
  std::chrono::steady_clock::time_point last_trigger_time;

  HOOKABLE(ActionTrigger, HOOK(event_trigger), HOOK(event_to_emit))
};
