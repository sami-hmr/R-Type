#pragma once

#include <utility>

#include "ByteParser/ByteParser.hpp"
#include "Json/JsonParser.hpp"
#include "ecs/Registry.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/components/Item.hpp"

struct PickUp
{
  Item item;
  std::size_t quantity;
  Registry::Entity possessor;

  PickUp(Item item, std::size_t quantity, Registry::Entity possessor)
      : item(std::move(item))
      , quantity(quantity)
      , possessor(possessor)
  {
  }

  PickUp(Registry& r, JsonObject const& e)
      : item(get_value_copy<Item>(r, e, "item").value())
      , quantity(get_value_copy<std::size_t>(r, e, "quantity").value())
      , possessor(static_cast<Registry::Entity>(
            get_value_copy<Registry::Entity>(r, e, "possessor").value()))
  {
  }

  CHANGE_ENTITY_DEFAULT

  DEFAULT_BYTE_CONSTRUCTOR(
      PickUp,
      ([](Item item, std::size_t quantity, Registry::Entity possessor)
       { return PickUp(std::move(item), quantity, possessor); }),
      parseByteItem(),
      parseByte<std::size_t>(),
      parseByte<Registry::Entity>())

  DEFAULT_SERIALIZE(item_to_byte(item),
                    type_to_byte(quantity),
                    type_to_byte(possessor))
};

template<typename Type>
struct ItemEvent
{
  std::uint8_t slot_item;
  bool usable;
  std::size_t nb_to_use;
  Registry::Entity consumer;

  ItemEvent(std::uint8_t slot_item,
            bool usable,
            std::size_t nb_to_use,
            Registry::Entity consumer)
      : slot_item(slot_item)
      , usable(usable)
      , nb_to_use(nb_to_use)
      , consumer(consumer)
  {
  }

  ItemEvent(Registry& r, JsonObject const& e)
      : slot_item(get_value_copy<std::uint8_t>(r, e, "slot_item").value())
      , usable(get_value_copy<bool>(r, e, "usable").value())
      , nb_to_use(get_value_copy<std::size_t>(r, e, "nb_to_use").value())
      , consumer(static_cast<Registry::Entity>(
            get_value_copy<Registry::Entity>(r, e, "consumer").value()))
  {
  }

  CHANGE_ENTITY_DEFAULT

  DEFAULT_BYTE_CONSTRUCTOR(
      ItemEvent,
      ([](std::uint8_t slot_item,
          bool usable,
          std::size_t nb_to_use,
          Registry::Entity consumer)
       { return ItemEvent(slot_item, usable, nb_to_use, consumer); }),
      parseByte<std::uint8_t>(),
      parseByte<bool>(),
      parseByte<std::size_t>(),
      parseByte<Registry::Entity>())

  DEFAULT_SERIALIZE(type_to_byte(slot_item),
                    type_to_byte(usable),
                    type_to_byte(nb_to_use),
                    type_to_byte(consumer))
};

struct ThrowItem
{
};

struct ConsumeItem
{
};

struct RemoveItem
{
};

using Throw = ItemEvent<ThrowItem>;
using Remove = ItemEvent<RemoveItem>;
using Consume = ItemEvent<ConsumeItem>;
