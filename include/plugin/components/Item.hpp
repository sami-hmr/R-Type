#pragma once

#include <string>
#include <sys/types.h>

#include "ByteParser/ByteParser.hpp"
#include "plugin/Byte.hpp"
#include "plugin/HookMacros.hpp"
#include "plugin/events/EventMacros.hpp"

struct Item
{
  std::string object;
  bool consumable;
  bool throwable;

  Item(std::string object, bool consumable, bool throwable)
      : object(std::move(object))
      , consumable(consumable)
      , throwable(throwable)
  {
  }

  bool operator<(const Item& other) const {
    return (this->object < other.object);
  }

  HOOKABLE(Item,
           HOOK(object),
           HOOK(consumable),
           HOOK(throwable))

  CHANGE_ENTITY_DEFAULT

  DEFAULT_BYTE_CONSTRUCTOR(Item,
      ([](std::string object, bool consumable, bool throwable)
       { return Item(std::move(object), consumable, throwable); }),
      parseByteString(),
      parseByte<bool>(),
      parseByte<bool>())

  DEFAULT_SERIALIZE(string_to_byte(this->object),
                    type_to_byte(this->consumable),
                    type_to_byte(this->throwable))
};
