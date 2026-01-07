#pragma once

#include <utility>

#include "ByteParser/ByteParser.hpp"
#include "Json/JsonParser.hpp"
#include "ecs/Registry.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"

// ByteArray item_to_byte(std::uint8_t i, std::endian endian = std::endian::big)
// {
//   return pair_to_byte<std::string, JsonObject>(
//              i.object, string_to_byte, json_object_to_byte)
//       + type_to_byte(i.consumable) + type_to_byte(i.throwable);
// }

// Parser<std::uint8_t> parseByteItem()
// {
//   return apply([](const std::pair<std::string, JsonObject>& p, bool c, bool t)
//                { return std::uint8_t(p, c, t); },
//                parseBytePair(parseByteString(), parseByteJsonObject()),
//                parseByte<bool>(),
//                parseByte<bool>());
// }

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

// struct PickUpItem
// {
// };
struct RemoveItem
{
};

using Throw = ItemEvent<ThrowItem>;
using Remove = ItemEvent<RemoveItem>;
// using PickUp = ItemEvent<PickUpItem>;
using Consume = ItemEvent<ConsumeItem>;
