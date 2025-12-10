#pragma once
#include "ByteParser/ByteParser.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"

struct Damage
{
  Damage() = default;

  Damage(int amount)
      : amount(amount)
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(Damage,
                           ([](int x) { return (Damage) {x}; }),
                           parseByte<int>())
  DEFAULT_SERIALIZE(type_to_byte(this->amount))

  CHANGE_ENTITY_DEFAULT

  int amount;

  HOOKABLE(Damage, HOOK(amount))
};
