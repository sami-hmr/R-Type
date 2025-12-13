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
#include "plugin/events/EventMacros.hpp"

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
                std::vector<std::pair<std::string, JsonObject>> const& evt_emt,
                bool triggered = false)
      : event_trigger(evt_trg)
      , event_to_emit(evt_emt)
      , triggered(triggered)
      , last_trigger_time(std::chrono::steady_clock::now())
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(
      ActionTrigger,
      ([](std::pair<std::string, JsonObject> const& trigger,
          std::vector<std::pair<std::string, JsonObject>> const& events,
          bool triggered)
       { return ActionTrigger(trigger, events, triggered); }),
      parseBytePair(parseByteString(), parseByteJsonObject()),
      parseByteArray(parseBytePair(parseByteString(), parseByteJsonObject())),
      parseByte<bool>())

  DEFAULT_SERIALIZE(
      pair_to_byte(event_trigger,
                   std::function<ByteArray(const std::string&)>(
                       [](const std::string& e) { return string_to_byte(e); }),
                   std::function<ByteArray(const JsonObject&)>(
                       [](const JsonObject& e)
                       { return json_object_to_byte(e); })),
      vector_to_byte(
          event_to_emit,
          std::function<ByteArray(const std::pair<std::string, JsonObject>&)>(
              [](const std::pair<std::string, JsonObject>& p)
              {
                return pair_to_byte(
                    p,
                    std::function<ByteArray(const std::string&)>(
                        [](std::string const& s) { return string_to_byte(s); }),
                    std::function<ByteArray(const JsonObject&)>(
                        [](JsonObject const& s)
                        { return json_object_to_byte(s); }));
              })),
      type_to_byte(triggered))

  CHANGE_ENTITY_DEFAULT

  std::pair<std::string, JsonObject> event_trigger;
  std::vector<std::pair<std::string, JsonObject>> event_to_emit;
  bool triggered;
  std::chrono::steady_clock::time_point last_trigger_time;

  HOOKABLE(ActionTrigger, HOOK(event_trigger), HOOK(event_to_emit))
};
