#include <chrono>
#include <format>
#include <iostream>

#include "Logger.hpp"

#include "ecs/EventManager.hpp"
#include "ecs/Registry.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/events/LoggerEvent.hpp"
#include "plugin/events/ShutdownEvent.hpp"

Logger::Logger(Registry& r,
               EventManager& em,
               EntityLoader& l,
               std::optional<JsonObject> const& config)
    : APlugin("logger", r, em, l, {}, {}, config)
    , _min_log_level(LogLevel::INFO)
{
  _log_file.open("rtype.log", std::ios::app);
  if (!_log_file.is_open()) {
    std::cerr << "Failed to open log file\n";
  }

  // Parse config to set log level
  if (config.has_value()) {
    try {
      if (config->contains("level")) {
        std::string level_str =
            std::get<std::string>(config->at("level").value);
        _min_log_level = LOG_LEVEL_STR.at_first(level_str);
      }
    } catch (std::bad_variant_access const&) {
      std::cerr << "Error parsing logger config: unexpected value type\n";
    } catch (std::out_of_range const&) {
      std::cerr << "Error parsing logger config: missing value\n";
    }
  }

  SUBSCRIBE_EVENT(LogEvent, { this->on_log_event(event); })
  SUBSCRIBE_EVENT(ShutdownEvent, {
    this->_event_manager.get().emit<LogEvent>(
        "System",
        event.exit_code == 0 ? LogLevel::INFO : LogLevel::WARNING,
        std::format(
            "Shutdown: {} (exit code: {})", event.reason, event.exit_code));
  })
}

void Logger::on_log_event(const LogEvent& event)
{
  // Filter events based on minimum log level
  if (event.level < _min_log_level) {
    return;
  }

  std::string timestamp = get_timestamp();
  std::string level_str = level_to_string(event.level);

  std::string log_message = std::format(
      "[{}] [{}] [{}] {}\n", timestamp, level_str, event.name, event.message);

  std::string color_code;
  std::string reset_code = "\033[0m";

  switch (event.level) {
    case LogLevel::DEBUG:
      color_code = "\033[36m";
      break;
    case LogLevel::INFO:
      color_code = "\033[32m";
      break;
    case LogLevel::WARNING:
      color_code = "\033[33m";
      break;
    case LogLevel::ERR:
      color_code = "\033[31m";
      break;
    default:
      color_code = "";
      reset_code = "";
  }

  std::cout << color_code << log_message << reset_code;

  if (_log_file.is_open()) {
    _log_file << log_message;
    _log_file.flush();
  }
}

std::string Logger::get_timestamp()
{
  auto now = std::chrono::system_clock::now();
  return std::format("{:%Y-%m-%d %H:%M:%S}", now);
}

std::string Logger::level_to_string(LogLevel level)
{
  switch (level) {
    case LogLevel::DEBUG:
      return "DEBUG";
    case LogLevel::INFO:
      return "INFO";
    case LogLevel::WARNING:
      return "WARNING";
    case LogLevel::ERR:
      return "ERROR";
    default:
      return "UNKNOWN";
  }
}

extern "C"
{
PLUGIN_EXPORT void* entry_point(Registry& r,
                  EventManager& em,
                  EntityLoader& e,
                  std::optional<JsonObject> const& config)
{
  return new Logger(r, em, e, config);
}
}
