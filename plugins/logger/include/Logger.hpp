#pragma once

#include <cstdint>
#include <fstream>
#include <string>
#include <utility>
#include <vector>

#include "Json/JsonParser.hpp"
#include "ecs/Registery.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"

/**
 * @brief Severity levels for logging.
 */
enum class LogLevel : std::uint8_t
{
  DEBUG,
  INFO,
  WARNING,
  ERROR
};

/**
 * @brief Event carrying log message data.
 */
struct LogEvent
{
  std::string name;
  LogLevel level;
  std::string message;
};

/**
 * @brief Component that marks an entity as loggable.
 */
struct LogComponent
{
  LogComponent(std::string name, LogLevel level = LogLevel::INFO)
      : name(std::move(name))
      , level(level)
  {
  }

  std::string name;
  LogLevel level;
};

/**
 * @brief Plugin that handles logging operations to file.
 */
class Logger : public APlugin
{
public:
  Logger(Registery& r, EntityLoader& l);

private:
  void init_log(Registery::Entity entity, JsonVariant const& config);

  void on_log_event(const LogEvent& event);

  static std::string get_timestamp();
  static std::string level_to_string(LogLevel level);

  std::ofstream _log_file;
  const std::vector<std::string> depends_on;
};
