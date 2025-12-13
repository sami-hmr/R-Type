#pragma once

#include <cstddef>
#include <mutex>
#include <optional>
#include <queue>
#include <string>
#include <tuple>
#include <unordered_map>

#include "ByteParser/ByteParser.hpp"
#include "ParserUtils.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/events/EventMacros.hpp"

struct ComponentBuilder
{
  std::size_t entity;
  std::string id;
  ByteArray data;

  ComponentBuilder() = default;

  ComponentBuilder(std::size_t e, std::string i, ByteArray d)
      : entity(e)
      , id(std::move(i))
      , data(std::move(d))
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(
      ComponentBuilder,
      ([](std::size_t e, std::string const& i, ByteArray const& d)
       { return ComponentBuilder(e, i, d); }),
      parseByte<std::size_t>(),
      parseByteString(),
      parseByte<Byte>().many())

  DEFAULT_SERIALIZE(type_to_byte<std::size_t>(this->entity),
                    string_to_byte(this->id),
                    this->data)

  CHANGE_ENTITY_DEFAULT

  ComponentBuilder(Registry& r, JsonObject const& e)
      : entity(get_value_copy<std::size_t>(r, e, "entity").value())
      , id(get_value_copy<std::string>(r, e, "id").value())
      , data(get_value_copy<Byte>(r, e, "data").value())
  {
  }
};

struct ComponentBuilderId
{
  std::optional<std::size_t> client;
  ComponentBuilder component;

  ComponentBuilderId() = default;

  ComponentBuilderId(std::optional<std::size_t> const& c,
                     ComponentBuilder component)
      : client(c)
      , component(std::move(component))
  {
  }

  ComponentBuilderId(std::optional<std::size_t> const& c,
                     std::size_t e,
                     std::string const& i,
                     ByteArray const& d)
      : client(c)
      , component(e, i, d)
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(ComponentBuilderId,
                           ([](std::optional<std::size_t> c,
                               std::size_t e,
                               std::string const& i,
                               ByteArray const& d)
                            { return ComponentBuilderId(c, e, i, d); }),
                           parseByteOptional(parseByte<std::size_t>()),
                           parseByte<std::size_t>(),
                           parseByteString(),
                           parseByte<Byte>().many())

  DEFAULT_SERIALIZE(optional_to_byte<std::size_t>(
                        client,
                        std::function<ByteArray(std::size_t const&)>(
                            [](std::size_t const& b)
                            { return type_to_byte(b); })),
                    component.to_bytes())

  CHANGE_ENTITY_DEFAULT

  ComponentBuilderId(Registry& r, JsonObject const& e)
      : client(get_value_copy<std::size_t>(r, e, "client").value())
      , component(get_value_copy<std::size_t>(r, e, "entity").value(),
                  get_value_copy<std::string>(r, e, "event_id").value(),
                  get_value_copy<ByteArray>(r, e, "data").value())
  {
  }
};

inline Parser<ComponentBuilder> parse_component_builder()
{
  return apply(
      [](std::size_t entity, std::string const& id, ByteArray const& data)
      { return ComponentBuilder(entity, std::move(id), std::move(data)); },
      parseByte<std::size_t>(),
      parseByteString(),
      parseByte<Byte>().many());
}

struct EventBuilder
{
  std::string event_id;
  ByteArray data;

  EventBuilder() = default;

  EventBuilder(std::string i, ByteArray d)
      : event_id(std::move(i))
      , data(std::move(d))
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(EventBuilder,
                           ([](std::string const& i, ByteArray const& d)
                            { return EventBuilder(i, d); }),
                           parseByteString(),
                           parseByte<Byte>().many())

  DEFAULT_SERIALIZE(string_to_byte(this->event_id), this->data)

  CHANGE_ENTITY_DEFAULT

  EventBuilder(Registry& r, JsonObject const& e)
      : event_id(get_value_copy<std::string>(r, e, "event_id").value())
      , data(get_value_copy<ByteArray>(r, e, "data").value())
  {
  }
};

inline Parser<EventBuilder> parse_event_builder()
{
  return apply([](std::string const& id, ByteArray const& data)
               { return EventBuilder(id, data); },
               parseByteString(),
               parseByte<Byte>().many());
}

struct EventBuilderId
{
  std::optional<std::size_t> client;
  EventBuilder event;

  EventBuilderId() = default;

  EventBuilderId(std::optional<std::size_t> const& c,
                 std::string const& i,
                 ByteArray const& d)
      : client(c)
      , event(i, d)
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(EventBuilderId,
                           ([](std::optional<std::size_t> c,
                               std::string const& i,
                               ByteArray const& d)
                            { return EventBuilderId(c, i, d); }),
                           parseByteOptional(parseByte<std::size_t>()),
                           parseByteString(),
                           parseByte<Byte>().many())

  DEFAULT_SERIALIZE(optional_to_byte<std::size_t>(
                        client,
                        std::function<ByteArray(std::size_t const&)>(
                            [](std::size_t const& b)
                            { return type_to_byte(b); })),
                    event.to_bytes())

  CHANGE_ENTITY_DEFAULT

  EventBuilderId(Registry& r, JsonObject const& e)
      : client(get_value_copy<std::size_t>(r, e, "client").value())
      , event(get_value_copy<std::string>(r, e, "event_id").value(),
              get_value_copy<ByteArray>(r, e, "data").value())
  {
  }
};

struct EntityCreation
{
  std::size_t client;

  EntityCreation() = default;

  EntityCreation(std::size_t c)
      : client(c)
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(EntityCreation,
                           ([](std::size_t c) { return EntityCreation(c); }),
                           parseByte<std::size_t>())

  DEFAULT_SERIALIZE(type_to_byte(client))

  CHANGE_ENTITY_DEFAULT

  EntityCreation(Registry& r, JsonObject const& e)
      : client(get_value_copy<std::size_t>(r, e, "client").value())
  {
  }
};

struct PlayerCreated
{
  std::size_t server_index;
  std::size_t client_id;

  PlayerCreated() = default;

  PlayerCreated(std::size_t server_index, std::size_t client_id)
      : server_index(server_index)
      , client_id(client_id)
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(PlayerCreated,
                           ([](std::size_t i, std::size_t id) { return PlayerCreated(i, id); }),
                           parseByte<std::size_t>(),
                           parseByte<std::size_t>())

  DEFAULT_SERIALIZE(type_to_byte(server_index), type_to_byte(client_id))

  CHANGE_ENTITY_DEFAULT

  PlayerCreated(Registry& r, JsonObject const& e)
      : server_index(get_value_copy<std::size_t>(r, e, "server_index").value())
  {
  }
};

template<typename T>
struct SharedQueue
{
  std::mutex lock;
  std::queue<T> queue;
};

template<typename K, typename V>
struct SharedMap
{
  std::mutex lock;
  std::unordered_map<K, V> map;
};

struct PlayerCreation
{
  std::size_t server_index;
  std::size_t server_id;

  PlayerCreation() = default;

  PlayerCreation(std::size_t server_index, std::size_t server_id)
      : server_index(server_index)
      , server_id(server_id)
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(PlayerCreation,
                           ([](std::size_t i, std::size_t id)
                            { return PlayerCreation(i, id); }),
                           parseByte<std::size_t>(),
                           parseByte<std::size_t>())

  DEFAULT_SERIALIZE(type_to_byte(server_index), type_to_byte(server_id))

  CHANGE_ENTITY_DEFAULT

  PlayerCreation(Registry& r, JsonObject const& e)
      : server_index(get_value_copy<std::size_t>(r, e, "server_index").value())
      , server_id(get_value_copy<std::size_t>(r, e, "server_id").value())
  {
  }
};
