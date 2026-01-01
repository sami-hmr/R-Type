#pragma once

#include "ByteParser/ByteParser.hpp"
#include "ecs/Registry.hpp"
#include "plugin/Byte.hpp"

template<typename Type>
struct ItemEvent
{
  std::string item;
  std::size_t nb_consumed;
  Registry::Entity consumer;

  ItemEvent(std::string item,
            std::size_t nb_consumed,
            Registry::Entity consumer)
      : item(std::move(item))
      , nb_consumed(nb_consumed)
      , consumer(consumer)
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(
      ItemEvent,
      ([](std::string item, std::size_t nb_consumed, Registry::Entity consumer)
       { return ItemEvent(item, nb_consumed, consumer); }),
      parseByteString(),
      parseByte<std::size_t>(),
      parseByte<Registry::Entity>())

  DEFAULT_SERIALIZE(string_to_byte(item),
                    type_to_byte(nb_consumed),
                    type_to_byte(consumer))
};

struct ThrownItem
{
};

struct ConsumedItem
{
};

struct RemovedItem
{
};

using Thrown = ItemEvent<ThrownItem>;
using Removed = ItemEvent<RemovedItem>;
using Consumed = ItemEvent<ConsumedItem>;
