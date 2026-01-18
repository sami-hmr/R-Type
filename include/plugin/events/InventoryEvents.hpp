#pragma once

#include <optional>
#include <utility>

#include "ByteParser/ByteParser.hpp"
#include "Json/JsonParser.hpp"
#include "ecs/Entity.hpp"
#include "ecs/Registry.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/components/Item.hpp"
#include "plugin/events/EventMacros.hpp"

struct PickUp
{
  Ecs::Entity to_pick;
  Ecs::Entity picker;

  PickUp(Ecs::Entity to_pick, Ecs::Entity picker)
      : to_pick(to_pick)
      , picker(picker)
  {
  }

  PickUp(Registry& r, JsonObject const& e, std::optional<Ecs::Entity> entity)
      : to_pick(get_value_copy<Ecs::Entity>(r, e, "to_pick", entity).value())
      , picker(get_value_copy<Ecs::Entity>(r, e, "picker", entity).value())
  {
  }

  CHANGE_ENTITY(result.to_pick = map.at(to_pick),
                result.to_pick = map.at(to_pick))

  DEFAULT_BYTE_CONSTRUCTOR(PickUp,
                           ([](Ecs::Entity to_pick, Ecs::Entity picker)
                            { return PickUp(to_pick, picker); }),
                           parseByte<Ecs::Entity>(),
                           parseByte<Ecs::Entity>())

  DEFAULT_SERIALIZE(type_to_byte(to_pick), type_to_byte(picker))
};

template<typename Type>
struct ItemEvent
{
  Ecs::Entity consumer;
  std::uint8_t slot_item;
  std::size_t nb_to_use;

  ItemEvent(Ecs::Entity consumer, std::uint8_t slot_item, std::size_t nb_to_use)
      : consumer(consumer)
      , slot_item(slot_item)
      , nb_to_use(nb_to_use)
  {
  }

  ItemEvent(Registry& r, JsonObject const& e, std::optional<Ecs::Entity> entity)
      : consumer(static_cast<Ecs::Entity>(
            get_value_copy<Ecs::Entity>(r, e, "consumer", entity).value()))
      , slot_item(
            get_value_copy<std::uint8_t>(r, e, "slot_item", entity).value())
      , nb_to_use(
            get_value_copy<std::size_t>(r, e, "nb_to_use", entity).value())
  {
  }

  CHANGE_ENTITY(result.consumer = map.at(consumer))

  DEFAULT_BYTE_CONSTRUCTOR(
      ItemEvent,
      ([](Ecs::Entity consumer, std::uint8_t slot_item, std::size_t nb_to_use)
       { return ItemEvent(consumer, slot_item, nb_to_use); }),
      parseByte<Ecs::Entity>(),
      parseByte<std::uint8_t>(),
      parseByte<std::size_t>())

  DEFAULT_SERIALIZE(type_to_byte(consumer),
                    type_to_byte(slot_item),
                    type_to_byte(nb_to_use))
};

struct Drop
{
};

struct Use
{
};

struct Remove
{
};

using DropItem = ItemEvent<Drop>;
using UseItem = ItemEvent<Use>;
using RemoveItem = ItemEvent<Remove>;
