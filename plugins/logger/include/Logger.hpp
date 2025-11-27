#pragma once

#include <fstream>
#include <string>
#include <utility>
#include <vector>

#include "Events.hpp"
#include "Json/JsonParser.hpp"
#include "ecs/Registery.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"

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
