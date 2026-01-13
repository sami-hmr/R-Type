#pragma once

#include <cstdint>
#include <iostream>
#include <optional>
#include <string>
#include <utility>

#include "ByteParser/ByteParser.hpp"
#include "EventMacros.hpp"
#include "TwoWayMap.hpp"
#include "ecs/Registry.hpp"
#include "plugin/Byte.hpp"

enum class LogLevel : std::uint8_t
{
  DEBUG,
  INFO,
  WARNING,
  ERR
};

static const TwoWayMap<std::string, LogLevel> LOG_LEVEL_STR = {
    {"DEBUG", LogLevel::DEBUG},
    {"INFO", LogLevel::INFO},
    {"WARNING", LogLevel::WARNING},
    {"ERROR", LogLevel::ERR},
};

inline std::ostream& operator<<(std::ostream& os, LogLevel level)
{
  return os << LOG_LEVEL_STR.at_second(level);
}

#define LOGGER(category, level, message) \
  if (this->_loader.get().is_plugin_loaded("logger")) { \
    this->_event_manager.get().emit<LogEvent>(category, level, message); \
  } else { \
    std::cerr << "[" << (level) << "] " << (category) << ": " << (message) \
              << "\n"; \
  }

#define LOGGER_EVTLESS(level, category, message) \
  std::cerr << "[" << (level) << "] " << (category) << ": " << (message) << "\n"
