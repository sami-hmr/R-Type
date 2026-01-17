#pragma once

#include <cstdint>

#include "ByteParser/ByteParser.hpp"
#include "EventMacros.hpp"
#include "Json/JsonParser.hpp"
#include "ecs/Registry.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/events/IoEvents.hpp"

struct Rebind
{
  std::uint16_t key_to_replace;
  std::uint16_t replacement_key;

  Rebind(std::uint16_t key_to_replace, std::uint16_t replacement_key)
      : key_to_replace(key_to_replace)
      , replacement_key(replacement_key)
  {
  }

  Rebind(Registry& r, JsonObject const& e, std::optional<Ecs::Entity> entity)
      : key_to_replace(
            static_cast<std::uint32_t>(KEY_MAPPING.at_first(
                get_value_copy<std::string>(r, e, "key_to_replace", entity).value()))
            << 8)
      , replacement_key(
            static_cast<std::uint32_t>(KEY_MAPPING.at_first(
                get_value_copy<std::string>(r, e, "replacement_key", entity).value()))
            << 8)
  {
  }

  CHANGE_ENTITY_DEFAULT

  DEFAULT_BYTE_CONSTRUCTOR(
      Rebind,
      ([](std::uint16_t key_to_replace, std::uint16_t replacement_key)
       { return Rebind(key_to_replace, replacement_key); }),
      parseByte<std::uint16_t>(),
      parseByte<std::uint16_t>())

  DEFAULT_SERIALIZE(type_to_byte(key_to_replace), type_to_byte(replacement_key))
};
