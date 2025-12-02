#include <chrono>
#include <format>
#include <iostream>

#include "Logger.hpp"

#include "plugin/events/Events.hpp"
#include "ecs/Registery.hpp"
#include "plugin/EntityLoader.hpp"

Logger::Logger(Registery& r, EntityLoader& l)
    : APlugin(r, l, {}, {COMP_INIT(log, init_log)})
{
  _log_file.open("rtype.log", std::ios::app);
  if (!_log_file.is_open()) {
    std::cerr << "Failed to open log file\n";
  }

  this->_registery.get().register_component<LogComponent>("logger:log");

  this->_registery.get().on<LogEvent>([this](const LogEvent& event)
                                      { this->on_log_event(event); });
  this->_registery.get().on<ShutdownEvent>(
      [this](const ShutdownEvent& event)
      {
        this->_registery.get().emit<LogEvent>(
            "System",
            event.exit_code == 0 ? LogLevel::INFO : LogLevel::WARNING,
            std::format(
                "Shutdown: {} (exit code: {})", event.reason, event.exit_code));
      });
}

void Logger::init_log(Registery::Entity const entity, JsonVariant const& config)
{
  try {
    JsonObject obj = std::get<JsonObject>(config);
    std::string name = std::get<std::string>(obj.at("name").value);

    LogLevel level = LogLevel::INFO;
    if (obj.contains("level")) {
      std::string level_str = std::get<std::string>(obj.at("level").value);
      if (level_str == "DEBUG") {
        level = LogLevel::DEBUG;
      } else if (level_str == "INFO") {
        level = LogLevel::INFO;
      } else if (level_str == "WARNING") {
        level = LogLevel::WARNING;
      } else if (level_str == "ERROR") {
        level = LogLevel::ERROR;
      }
    }

    this->_registery.get().emplace_component<LogComponent>(entity, name, level);
  } catch (std::bad_variant_access const&) {
    std::cerr << "Error loading log component: unexpected value type\n";
  } catch (std::out_of_range const&) {
    std::cerr << "Error loading log component: missing value in JsonObject\n";
  }
}

void Logger::on_log_event(const LogEvent& event)
{
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
    case LogLevel::ERROR:
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
    case LogLevel::ERROR:
      return "ERROR";
    default:
      return "UNKNOWN";
  }
}

extern "C"
{
void* entry_point(Registery& r, EntityLoader& e)
{
  return new Logger(r, e);
}
}
