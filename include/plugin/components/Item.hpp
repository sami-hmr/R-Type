#pragma once

#include <string>

#include <sys/types.h>

#include "ByteParser/ByteParser.hpp"
#include "Json/JsonParser.hpp"
#include "ecs/Registry.hpp"
#include "plugin/Byte.hpp"
#include "plugin/HookMacros.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/events/EventMacros.hpp"

struct Item
{
  using Object = std::pair<std::string, JsonObject>;

  Object object = {"", {}};
  bool consumable;
  bool throwable;

  Item() = default;

  Item(std::pair<std::string, JsonObject> object,
       bool consumable,
       bool throwable)
      : object(std::move(object))
      , consumable(consumable)
      , throwable(throwable)
  {
  }

  bool operator<(const Item& other) const
  {
    return (this->object.first < other.object.first);
  }

  bool operator==(const Item& other) const
  {
    return (this->object.first == other.object.first);
  }

  HOOKABLE(Item, HOOK(object), HOOK(consumable), HOOK(throwable))

  CHANGE_ENTITY_DEFAULT

  DEFAULT_BYTE_CONSTRUCTOR(
      Item,
      ([](std::pair<std::string, JsonObject> object,
          bool consumable,
          bool throwable)
       { return Item(std::move(object), consumable, throwable); }),
      parseBytePair(parseByteString(), parseByteJsonObject()),
      parseByte<bool>(),
      parseByte<bool>())

  DEFAULT_SERIALIZE(
      SERIALIZE_FUNCTION<Object>(
          pair_to_byte<std::string, JsonObject>,
          SERIALIZE_FUNCTION<std::string>(string_to_byte),
          SERIALIZE_FUNCTION<JsonObject>(json_object_to_byte))(this->object),
      type_to_byte(this->consumable),
      type_to_byte(this->throwable))

  Item(Registry& r, JsonObject const& e)
      : object(
            std::make_pair(get_value_copy<std::string>(r, e, "name").value(),
                           get_value_copy<JsonObject>(r, e, "config").value()))
      , consumable(get_value_copy<bool>(r, e, "consumable").value())
      , throwable(get_value_copy<bool>(r, e, "throwable").value())
  {
  }
};

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
