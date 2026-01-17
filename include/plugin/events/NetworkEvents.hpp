#pragma once

#include <cstddef>

#include "ByteParser/ByteParser.hpp"
#include "Json/JsonParser.hpp"
#include "ecs/Registry.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"

struct StateTransfer
{
  StateTransfer() = default;

  std::size_t client_id;

  StateTransfer(std::size_t client)
      : client_id(client)
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(StateTransfer,
                           ([](std::size_t c) { return StateTransfer(c); }),
                           parseByte<std::size_t>())

  DEFAULT_SERIALIZE(type_to_byte(client_id))

  StateTransfer(Registry& r,
                JsonObject const& e,
                std::optional<Ecs::Entity> entity)
      : client_id(
            get_value_copy<std::size_t>(r, e, "client_id", entity).value())
  {
  }

  CHANGE_ENTITY_DEFAULT
};

struct PlayerReady
{
  PlayerReady() = default;

  std::size_t client_id;

  PlayerReady(std::size_t client)
      : client_id(client)
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(PlayerReady,
                           ([](std::size_t c) { return PlayerReady(c); }),
                           parseByte<std::size_t>())

  DEFAULT_SERIALIZE(type_to_byte(client_id))

  PlayerReady(Registry& r,
              JsonObject const& e,
              std::optional<Ecs::Entity> entity)
      : client_id(
            get_value_copy<std::size_t>(r, e, "client_id", entity).value())
  {
  }

  CHANGE_ENTITY_DEFAULT
};

struct WantReady
{
  WantReady() = default;

  EMPTY_BYTE_CONSTRUCTOR(WantReady)
  DEFAULT_SERIALIZE(ByteArray {})

  CHANGE_ENTITY_DEFAULT

  WantReady(Registry&, JsonObject const&, std::optional<Ecs::Entity>) {}

  HOOKABLE(WantReady)
};

struct Disconnection
{
  Disconnection() = default;

  EMPTY_BYTE_CONSTRUCTOR(Disconnection)
  DEFAULT_SERIALIZE(ByteArray {})

  CHANGE_ENTITY_DEFAULT

  Disconnection(Registry&, JsonObject const&, std::optional<Ecs::Entity>) {}

  HOOKABLE(Disconnection)
};

struct ResetClient
{
  ResetClient(std::size_t sequence)
      : sequence(sequence)
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(
      ResetClient,
      [](std::size_t s) { return ResetClient(s); },
      parseByte<std::size_t>())
  DEFAULT_SERIALIZE(type_to_byte(this->sequence))

  CHANGE_ENTITY_DEFAULT

  ResetClient(Registry&, JsonObject const&, std::optional<Ecs::Entity>) {}

  HOOKABLE(ResetClient)

  std::size_t sequence;
};
