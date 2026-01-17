#pragma once
#include "ByteParser/ByteParser.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"

struct Heal
{
  Heal() = default;

  Heal(int amount)
      : amount(amount)
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(Heal,
                           ([](int x) { return Heal {x}; }),
                           parseByte<int>())
  DEFAULT_SERIALIZE(type_to_byte(this->amount))

  CHANGE_ENTITY_DEFAULT

  int amount;

  HOOKABLE(Heal, HOOK(amount))
};
