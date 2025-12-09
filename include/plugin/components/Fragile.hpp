#pragma once

#include "ParserUtils.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/events/EventMacros.hpp"

struct Fragile
{
  Fragile() = default;

  EMPTY_BYTE_CONSTRUCTOR(Fragile)
  DEFAULT_SERIALIZE(ByteArray {})
  CHANGE_ENTITY_DEFAULT

  HOOKABLE(Fragile)
};
