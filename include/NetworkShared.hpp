#pragma once

#include <cstddef>
#include <mutex>
#include <optional>
#include <ostream>
#include <queue>
#include <semaphore>
#include <string>
#include <unordered_map>
#include <vector>

#include "ByteParser/ByteParser.hpp"
#include "ParserUtils.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/events/EventMacros.hpp"

struct ComponentBuilder
{
  std::size_t entity = 0;
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
      : client(get_value_copy<std::size_t>(r, e, "client"))
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
      { return ComponentBuilder(entity, id, data); },
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
      : client(get_value_copy<std::size_t>(r, e, "client"))
      , event(get_value_copy<std::string>(r, e, "event_id").value(),
              get_value_copy<ByteArray>(r, e, "data").value())
  {
  }
};

struct NewConnection
{
  std::size_t client = 0;
  int user_id = 0;

  NewConnection() = default;

  NewConnection(std::size_t c, int u)
      : client(c)
      , user_id(u)
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(NewConnection,
                           ([](std::size_t c, int u)
                            { return NewConnection(c, u); }),
                           parseByte<std::size_t>(),
                           parseByte<int>())

  DEFAULT_SERIALIZE(type_to_byte(client), type_to_byte(user_id))

  CHANGE_ENTITY_DEFAULT

  NewConnection(Registry& r, JsonObject const& e)
      : client(get_value_copy<std::size_t>(r, e, "client").value())
      , user_id(get_value_copy<int>(r, e, "user_id").value())
  {
  }
};

struct PlayerCreated
{
  std::size_t server_index = 0;
  std::size_t client_id = 0;

  PlayerCreated() = default;

  PlayerCreated(std::size_t server_index, std::size_t client_id)
      : server_index(server_index)
      , client_id(client_id)
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(PlayerCreated,
                           ([](std::size_t i, std::size_t id)
                            { return PlayerCreated(i, id); }),
                           parseByte<std::size_t>(),
                           parseByte<std::size_t>())

  DEFAULT_SERIALIZE(type_to_byte(server_index), type_to_byte(client_id))

  CHANGE_ENTITY_DEFAULT

  PlayerCreated(Registry& r, JsonObject const& e)
      : server_index(
            get_value_copy<Registry::Entity>(r, e, "server_index").value())
  {
  }
};

struct NetworkStatus
{
  enum PacketLossLevel : std::uint8_t
  {
    NONE,
    LOW,
    MEDIUM,
    HIGH,
  };

  std::size_t ping_in_millisecond = 0;
  PacketLossLevel packet_loss = NONE;

  NetworkStatus() = default;

  NetworkStatus(std::size_t ping, PacketLossLevel pl)
      : ping_in_millisecond(ping)
      , packet_loss(pl)
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(
      NetworkStatus,
      ([](std::size_t p, std::uint8_t pl)
       { return NetworkStatus(p, static_cast<PacketLossLevel>(pl)); }),
      parseByte<std::size_t>(),
      parseByte<std::uint8_t>())

  DEFAULT_SERIALIZE(type_to_byte(ping_in_millisecond),
                    type_to_byte(packet_loss))

  CHANGE_ENTITY_DEFAULT

  NetworkStatus(Registry& r, JsonObject const& e)
      : ping_in_millisecond(get_value_copy<int>(r, e, "ping").value())
      , packet_loss(static_cast<PacketLossLevel>(
            get_value_copy<int>(r, e, "packet_loss").value()))
  {
  }
};

inline std::ostream& operator<<(std::ostream& os,
                                NetworkStatus::PacketLossLevel level)
{
  switch (level) {
    case NetworkStatus::NONE:
      return os << "NONE";
    case NetworkStatus::LOW:
      return os << "LOW";
    case NetworkStatus::MEDIUM:
      return os << "MEDIUM";
    case NetworkStatus::HIGH:
      return os << "HIGH";
  }
  return os;
}

struct HearthBeat
{
  std::size_t send_timestamp = 0;
  std::vector<std::size_t> lost_packages;

  HearthBeat() = default;

  HearthBeat(std::size_t send_timestamp,
             std::vector<std::size_t> const& lost_packages)
      : send_timestamp(send_timestamp)
      , lost_packages(lost_packages)
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(HearthBeat,
                           ([](std::size_t st,
                               std::vector<std::size_t> const& lp)
                            { return HearthBeat(st, lp); }),
                           parseByte<std::size_t>(),
                           parseByteArray(parseByte<std::size_t>()))

  DEFAULT_SERIALIZE(type_to_byte(send_timestamp),
                    vector_to_byte(lost_packages, TTB_FUNCTION<std::size_t>()))

  CHANGE_ENTITY_DEFAULT

  HearthBeat(Registry& /*r*/, JsonObject const& /*e*/)  // TODO
  {
  }
};

struct DisconnectClient
{
  std::size_t client = 0;

  DisconnectClient() = default;

  DisconnectClient(std::size_t c)
      : client(c)
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(DisconnectClient,
                           ([](std::size_t c) { return DisconnectClient(c); }),
                           parseByte<std::size_t>())

  DEFAULT_SERIALIZE(type_to_byte(client))

  CHANGE_ENTITY_DEFAULT

  DisconnectClient(Registry& r, JsonObject const& e)
      : client(get_value_copy<std::size_t>(r, e, "client").value())
  {
  }
};

template<typename T>
struct SharedQueue
{
  SharedQueue()
      : semaphore(0)
  {
  }

  void push(T const& obj)
  {
    std::lock_guard<std::mutex> guard(this->lock);
    this->queue.push(obj);
    this->semaphore.release();
  }

  T pop()
  {
    std::lock_guard<std::mutex> guard(this->lock);
    auto tmp = this->queue.front();
    this->queue.pop();
    return std::move(tmp);
  }

  std::vector<T> flush()
  {
    std::lock_guard<std::mutex> guard(this->lock);
    std::vector<T> tmp;
    tmp.reserve(this->queue.size());
    while (!this->queue.empty()) {
      tmp.push_back(this->queue.front());
      this->queue.pop();
    }
    return std::move(tmp);
  }

  void wait() { this->semaphore.acquire(); }

  void release() { this->semaphore.release(); }

  std::mutex lock;
  std::counting_semaphore<> semaphore;
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
  std::size_t server_index = 0;
  std::size_t server_id = 0;

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
