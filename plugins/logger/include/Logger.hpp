#pragma once

#include <fstream>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "BaseTypes.hpp"
#include "ByteParser/ByteParser.hpp"
#include "Events.hpp"
#include "Json/JsonParser.hpp"
#include "ParserTypes.hpp"
#include "Rest.hpp"
#include "ecs/Registery.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/Byte.hpp"
#include "plugin/EntityLoader.hpp"

class Logger : public APlugin
{
public:
  Logger(Registery& r,
         EntityLoader& l,
         std::optional<JsonObject> const& config);

private:
  void on_log_event(const LogEvent& event);

  static std::string get_timestamp();
  static std::string level_to_string(LogLevel level);

  std::ofstream _log_file;
  LogLevel _min_log_level;
};
