#pragma once

#include <cstdint>
#include <string>

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
