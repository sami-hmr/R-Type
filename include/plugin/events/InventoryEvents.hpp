#pragma once

#include <utility>

#include "ByteParser/ByteParser.hpp"
#include "Json/JsonParser.hpp"
#include "ecs/Registry.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/components/Item.hpp"

ByteArray item_to_byte(Item i)
{
  return pair_to_byte<std::string, JsonObject>(
             i.object, string_to_byte, json_object_to_byte)
      + type_to_byte(i.consumable) + type_to_byte(i.throwable);
}

Parser<Item> parseByteItem()
{
  return apply([](const std::pair<std::string, JsonObject>& p, bool c, bool t)
               { return Item(p, c, t); },
               parseBytePair(parseByteString(), parseByteJsonObject()),
               parseByte<bool>(),
               parseByte<bool>());
}

struct PickUp
{
  Item item;
  bool usable;
  std::size_t nb_to_use;
  Registry::Entity consumer;

  PickUp(Item item,
         bool usable,
         std::size_t nb_to_use,
         Registry::Entity consumer)
      : item(std::move(item))
      , usable(usable)
      , nb_to_use(nb_to_use)
      , consumer(consumer)
  {
  }

  PickUp(Registry& r, JsonObject const& e)
      : item(get_value_copy<Item>(r, e, "item").value())
      , usable(get_value_copy<bool>(r, e, "usable").value())
      , nb_to_use(get_value_copy<std::size_t>(r, e, "nb_to_use").value())
      , consumer(static_cast<Registry::Entity>(
            get_value_copy<Registry::Entity>(r, e, "consumer").value()))
  {
  }

  CHANGE_ENTITY_DEFAULT

  DEFAULT_BYTE_CONSTRUCTOR(
      PickUp,
      ([](Item item,
          bool usable,
          std::size_t nb_to_use,
          Registry::Entity consumer)
       { return PickUp(std::move(item), usable, nb_to_use, consumer); }),
      parseByteItem(),
      parseByte<bool>(),
      parseByte<std::size_t>(),
      parseByte<Registry::Entity>())

  DEFAULT_SERIALIZE(item_to_byte(item),
                    type_to_byte(usable),
                    type_to_byte(nb_to_use),
                    type_to_byte(consumer))
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
