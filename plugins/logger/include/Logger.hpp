#pragma once

#include <fstream>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "BaseTypes.hpp"
#include "ByteParser/ByteParser.hpp"
#include "Json/JsonParser.hpp"
#include "ParserTypes.hpp"
#include "Rest.hpp"
#include "ecs/Registry.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/Byte.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/events/Log.hpp"
#include "plugin/events/Events.hpp"
#include "plugin/events/LoggerEvent.hpp"

struct LogComponent
{
  LogComponent() = default;
  DEFAULT_BYTE_CONSTRUCTOR(
      LogComponent,
      ([](std::vector<char> name, LogLevel level)
       { return LogComponent(std::string(name.begin(), name.end()), level); }),
      parseByteArray(parseAnyChar()),
      parseByte<LogLevel>())
  DEFAULT_SERIALIZE(string_to_byte(this->name),
                    type_to_byte((uint8_t)this->level))

  LogComponent(std::string name, LogLevel level = LogLevel::INFO)
      : name(std::move(name))
      , level(level)
  {
  }

  std::string name;
  LogLevel level = LogLevel::DEBUG;
};

class Logger : public APlugin
{
public:
  Logger(Registry& r, EntityLoader& l, std::optional<JsonObject> const& config);

private:
  void on_log_event(const LogEvent& event);

  static std::string get_timestamp();
  static std::string level_to_string(LogLevel level);

  std::ofstream _log_file;
  LogLevel _min_log_level;
};
