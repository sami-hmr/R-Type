#pragma once

#include <string>
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

  WaveSpawnEvent(std::string wave_temp)
      : wave_template(std::move(wave_temp))
  {
  }

  CHANGE_ENTITY_DEFAULT

  DEFAULT_BYTE_CONSTRUCTOR(WaveSpawnEvent,
                           ([](std::string const& wave_temp)
                            { return WaveSpawnEvent(wave_temp); }),
                           parseByteString())

  DEFAULT_SERIALIZE(string_to_byte(wave_template))

  WaveSpawnEvent(Registry& r, JsonObject const& conf)
      : wave_template(*get_value_copy<std::string>(r, conf, "wave_template"))
  {
  }

  std::string wave_template;
};
