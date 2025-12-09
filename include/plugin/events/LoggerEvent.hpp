#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <utility>

#include "ByteParser/ByteParser.hpp"
#include "EventMacros.hpp"
#include "TwoWayMap.hpp"
#include "ecs/Registry.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"

enum class LogLevel : std::uint8_t
{
  DEBUG,
  INFO,
  WARNING,
  ERROR
};
static const TwoWayMap<std::string, LogLevel> LOG_LEVEL_STR = {
    {"DEBUG", LogLevel::DEBUG},
    {"INFO", LogLevel::INFO},
    {"WARNING", LogLevel::WARNING},
    {"ERROR", LogLevel::ERROR},
};

struct LogEvent
{
  std::string name;
  LogLevel level;
  std::string message;

  CHANGE_ENTITY_DEFAULT

  LogEvent(std::string n, LogLevel l, std::string m)
      : name(std::move(n))
      , level(l)
      , message(std::move(m))
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(LogEvent,
                         ([](std::string const& n, LogLevel l, std::string const& m)
                          { return (LogEvent) {n, l, m}; }),
                         parseByteString(),
                         parseByte<LogLevel>(),
                         parseByteString())

  DEFAULT_SERIALIZE(string_to_byte(this->name), type_to_byte(this->level), string_to_byte(this->message))


  LogEvent(Registry& r, JsonObject const& e)
      : name(get_value_copy<std::string>(r, e, "name").value())
      , level(static_cast<LogLevel>(LOG_LEVEL_STR.at_first(
            get_value_copy<std::string>(r, e, "level").value())))
      , message(get_value_copy<std::string>(r, e, "message").value())
  {
  }
};

#define LOGGER(category, level, message) \
  this->_registry.get().emit<LogEvent>(category, level, message);
