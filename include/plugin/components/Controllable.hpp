#pragma once

#include <cstdint>
#include <unordered_map>
#include <utility>

#include "ByteParser/ByteParser.hpp"
#include "Json/JsonParser.hpp"
#include "plugin/Byte.hpp"
#include "plugin/HookMacros.hpp"
#include "plugin/events/EventMacros.hpp"

enum KeyEventType : bool
{
  KEY_RELEASED = false,
  KEY_PRESSED = true,
};

struct Controllable
{
  using Descript = std::pair<std::string, std::string>;
  using Trigger = std::pair<Descript, JsonObject>;
  Controllable() = default;

  Controllable(std::unordered_map<std::uint16_t, Trigger> map)
      : event_map(std::move(map))
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(
      Controllable,
      ([](std::unordered_map<std::uint16_t, Trigger> const& map)
       { return Controllable(map); }),
      parseByteMap(parseByte<std::uint16_t>(),
                   parseBytePair(parseBytePair(parseByteString(),
                                               parseByteString()),
                                 parseByteJsonObject())))

  DEFAULT_SERIALIZE(
      map_to_byte(event_map,
                  TTB_FUNCTION<std::uint16_t>(),
                  SERIALIZE_FUNCTION<Trigger>(
                      pair_to_byte<Descript, JsonObject>,
                      SERIALIZE_FUNCTION<Descript>(
                          pair_to_byte<std::string, std::string>,
                          SERIALIZE_FUNCTION<std::string>(string_to_byte),
                          SERIALIZE_FUNCTION<std::string>(string_to_byte)),
                      SERIALIZE_FUNCTION<JsonObject>(json_object_to_byte))))
  CHANGE_ENTITY_DEFAULT

  std::unordered_map<std::uint16_t, Trigger> event_map;

  HOOKABLE(Controllable)

};
