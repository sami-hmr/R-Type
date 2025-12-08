#pragma once

#include <cstddef>
#include <mutex>
#include <queue>
#include <string>
#include <tuple>

#include "ByteParser/ByteParser.hpp"
#include "ParserUtils.hpp"
#include "plugin/Byte.hpp"

struct ComponentBuilder
{
  std::size_t entity;
  std::string id;
  ByteArray data;
};

inline Parser<ComponentBuilder> parse_component_builder()
{
  return apply(
      [](std::size_t entity, std::string id, ByteArray data)
      {
        return ComponentBuilder {
            .entity = entity, .id = std::move(id), .data = std::move(data)};
      },
      parseByte<std::size_t>(),
      parseByteString(),
      parseByte<Byte>().many());
}

struct EventBuilder
{
  std::string event_id;
  ByteArray data;
};

inline Parser<EventBuilder> parse_event_builder()
{
  return apply(
      [](std::string id, ByteArray data)
      {
        return EventBuilder {.event_id = std::move(id),
                             .data = std::move(data)};
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
