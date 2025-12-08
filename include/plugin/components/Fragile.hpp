#pragma once

#include "ParserUtils.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"

struct Fragile
{
  Fragile() = default;

  EMPTY_BYTE_CONSTRUCTOR(Fragile)
  DEFAULT_SERIALIZE(ByteArray {})

  HOOKABLE(Fragile)
};
