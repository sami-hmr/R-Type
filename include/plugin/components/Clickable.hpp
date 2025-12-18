#pragma once

#include <string>
#include <utility>
#include <vector>

#include "ByteParser/ByteParser.hpp"
#include "Json/JsonParser.hpp"
#include "plugin/Byte.hpp"
#include "plugin/HookMacros.hpp"
#include "plugin/events/EventMacros.hpp"

struct Clickable
{
  std::vector<std::pair<std::string, JsonObject>> to_emit;

  Clickable() = default;

  Clickable(std::vector<std::pair<std::string, JsonObject>> emits)
      : to_emit(std::move(emits))
  {
  }

  CHANGE_ENTITY_DEFAULT

  DEFAULT_BYTE_CONSTRUCTOR(
      Clickable,
      [](std::vector<std::pair<std::string, JsonObject>> to_emit)
      { return Clickable(std::move(to_emit)); },
      parseByteArray(parseBytePair(parseByteString(), parseByteJsonObject())))

  DEFAULT_SERIALIZE(vector_to_byte(
      to_emit,
      std::function<ByteArray(const std::pair<std::string, JsonObject>&)>(
          [](const std::pair<std::string, JsonObject>& p)
          {
            return pair_to_byte(
                p,
                std::function<ByteArray(const std::string&)>(
                    [](std::string const& s) { return string_to_byte(s); }),
                std::function<ByteArray(const JsonObject&)>(
                    [](JsonObject const& s)
                    { return json_object_to_byte(s); }));
          })))

  HOOKABLE(Clickable, HOOK(to_emit));
};
