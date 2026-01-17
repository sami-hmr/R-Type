#pragma once

#include <string>
#include <vector>

#include "ByteParser/ByteParser.hpp"
#include "EventMacros.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"

struct CreateEntity
{
  using Additional = std::vector<std::pair<std::string, ByteArray>>;

  CreateEntity(Additional additionals)
      : additionals(std::move(additionals))
  {
  }

  CHANGE_ENTITY_DEFAULT

  DEFAULT_BYTE_CONSTRUCTOR(
      CreateEntity,
      ([](Additional const& a) { return CreateEntity(a); }),
      parseByteArray(parseBytePair(parseByteString(),
                                   parseByteArray(parseByte<Byte>()))))

  DEFAULT_SERIALIZE(vector_to_byte(
      additionals,
      std::function<ByteArray(const std::pair<std::string, ByteArray>&)>(
          [](const std::pair<std::string, ByteArray>& pair)
          {
            return pair_to_byte(
                pair,
                std::function<ByteArray(const std::string&)>(
                    [](const std::string& s) { return string_to_byte(s); }),
                std::function<ByteArray(const ByteArray&)>(
                    [](ByteArray const& b)
                    {
                      return vector_to_byte(
                          b,
                          std::function<ByteArray(const Byte&)>(
                              [](Byte const& b)
                              { return type_to_byte<Byte>(b); }));
                    }));
          })))

  CreateEntity(Registry& r, JsonObject const& e) {}

  Additional additionals;
};
