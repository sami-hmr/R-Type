#pragma once

#include <cstddef>
#include <cstdint>

#include "ByteParser/ByteParser.hpp"
#include "EventMacros.hpp"
#include "Json/JsonParser.hpp"
#include "ecs/Registry.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/events/IoEvents.hpp"

struct Rebind
{
  Registry::Entity entity;
  std::uint16_t key_to_replace;
  std::uint16_t replacement_key;

  Rebind(Registry::Entity entity,
         std::uint16_t key_to_replace,
         std::uint16_t replacement_key)
      : entity(entity)
      , key_to_replace(key_to_replace)
      , replacement_key(replacement_key)
  {
  }

  Rebind(Registry& r, JsonObject const& e)
      : entity(get_value_copy<Registry::Entity>(r, e, "key_to_replace").value())
      , key_to_replace(
            static_cast<std::uint32_t>(KEY_MAPPING.at_first(
                get_value_copy<std::string>(r, e, "key_to_replace").value()))
            << 8)
      , replacement_key(
            static_cast<std::uint32_t>(KEY_MAPPING.at_first(
                get_value_copy<std::string>(r, e, "replacement_key").value()))
            << 8)
  {
  }

  CHANGE_ENTITY_DEFAULT

  DEFAULT_BYTE_CONSTRUCTOR(
      Rebind,  // NOLINT
      ([](Registry::Entity entity,
          std::uint16_t key_to_replace,
          std::uint16_t replacement_key)
       { return Rebind(entity, key_to_replace, replacement_key); }),
      parseByte<Registry::Entity>(),
      parseByte<std::uint16_t>(),
      parseByte<std::uint16_t>())

  DEFAULT_SERIALIZE(type_to_byte(entity),
                    type_to_byte(key_to_replace),
                    type_to_byte(replacement_key))
};

struct GenerateRebindingScene
{
  GenerateRebindingScene() = default;

  GenerateRebindingScene(std::size_t e, std::string t)
      : entity(e)
      , scene_name(std::move(t))
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(GenerateRebindingScene,
                           ([](std::size_t e, std::string const& t)
                            { return GenerateRebindingScene(e, t); }),
                           parseByte<std::size_t>(),
                           parseByteString())

  DEFAULT_SERIALIZE(type_to_byte(entity), string_to_byte(this->scene_name))

  GenerateRebindingScene(Registry& r, JsonObject const& e)
      : entity(get_value_copy<Registry::Entity>(r, e, "entity").value_or(0))
      , scene_name(get_value_copy<std::string>(r, e, "scene_name").value_or(""))
  {
  }

  CHANGE_ENTITY_DEFAULT

  std::size_t entity = 0;
  std::string scene_name;
};

struct WatchRebind
{
  WatchRebind() = default;

  WatchRebind(Registry::Entity entity, std::uint16_t key)
      : entity(entity)
      , key(key)
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(WatchRebind,
                           ([](Registry::Entity entity, std::uint16_t k)
                            { return WatchRebind(entity, k); }),
                           parseByte<Registry::Entity>(),
                           parseByte<std::uint16_t>())

  DEFAULT_SERIALIZE(type_to_byte(key))

  WatchRebind(Registry& r, JsonObject const& e)
      : entity(get_value_copy<int>(r, e, "entity").value_or(0))
      , key(get_value_copy<int>(r, e, "key").value_or(0))
  {
  }

  CHANGE_ENTITY_DEFAULT

  Registry::Entity entity;
  std::uint16_t key;
};

struct ExitRebind
{
  ExitRebind() = default;

  EMPTY_BYTE_CONSTRUCTOR(ExitRebind)
  DEFAULT_SERIALIZE(ByteArray {})

  CHANGE_ENTITY_DEFAULT

  ExitRebind(Registry&, JsonObject const&) {}

  HOOKABLE(ExitRebind)
};
