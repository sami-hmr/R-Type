#pragma once

#include <cstddef>
#include <mutex>
#include <queue>
#include <string>
#include <tuple>

#include "ByteParser/ByteParser.hpp"
#include "ParserUtils.hpp"
#include "ecs/Registry.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"

struct ComponentBuilder
{
  std::size_t entity;
  std::string id;
  ByteArray data;

  ComponentBuilder() = default;

  ComponentBuilder(std::size_t e, std::string i, ByteArray d)
    : entity(e)
    , id(std::move(i))
    , data(std::move(d)) {}

  DEFAULT_BYTE_CONSTRUCTOR(ComponentBuilder,
                         ([](std::size_t e, std::string const &i, ByteArray const &d)
                          { return ComponentBuilder(e, i, d); }),
                         parseByte<std::size_t>(),
                         parseByteString(),
                         parseByte<Byte>().many())

  DEFAULT_SERIALIZE(type_to_byte<std::size_t>(this->entity), string_to_byte(this->id), this->data)

  CHANGE_ENTITY_DEFAULT

  ComponentBuilder(Registry& r, JsonObject const& e)
      : entity(get_value_copy<std::size_t>(r, e, "entity").value())
      , id(get_value_copy<std::string>(r, e, "id").value())
      , data(get_value_copy<Byte>(r, e, "data").value())
  {
  }
};

inline Parser<ComponentBuilder> parse_component_builder()
{
  return apply(
      [](std::size_t entity, std::string const &id, ByteArray const &data)
      {
        return ComponentBuilder(
             entity, std::move(id), std::move(data));
      },
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
    , data(std::move(d)) {}

  DEFAULT_BYTE_CONSTRUCTOR(EventBuilder,
                         ([](std::string const &i, ByteArray const &d)
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
  return apply(
      [](std::string const &id, ByteArray const &data)
      {
        return EventBuilder(id, data);
      },
      parseByteString(),
      parseByte<Byte>().many());
}

template<typename T>
struct SharedQueue
{
  std::mutex lock;
  std::queue<T> queue;
};
