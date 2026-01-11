#pragma once

#include <string>
#include <vector>
#include <utility>

#include "ByteParser/ByteParser.hpp"
#include "Json/JsonParser.hpp"
#include "ParserUtils.hpp"
#include "ecs/Registry.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/events/EventMacros.hpp"

struct WaveSpawnEvent
{
  WaveSpawnEvent() = default;

  WaveSpawnEvent(std::vector<std::string> const& wave_temps)
      : wave_templates(wave_temps)
  {
  }

  CHANGE_ENTITY_DEFAULT

  DEFAULT_BYTE_CONSTRUCTOR(WaveSpawnEvent,
                           ([](std::vector<std::string> const& wave_temps)
                            { return WaveSpawnEvent(wave_temps); }),
                           parseByteArray(parseByteString()))

  DEFAULT_SERIALIZE(vector_to_byte(
      wave_templates,
      std::function<ByteArray(const std::string&)>(
          [](const std::string& s) { return string_to_byte(s); })))

  WaveSpawnEvent(Registry&  /*r*/, JsonObject const& conf)
  {
    if (conf.contains("wave_templates")) {
      auto const& wave_array = std::get<JsonArray>(conf.at("wave_templates").value);
      for (auto const& item : wave_array) {
        wave_templates.push_back(std::get<std::string>(item.value));
      }
    }
  }

  std::vector<std::string> wave_templates;
};
