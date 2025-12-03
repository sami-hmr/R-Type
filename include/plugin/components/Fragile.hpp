#pragma once

#include "ParserUtils.hpp"
#include "plugin/Byte.hpp"

struct Fragile
{
  Fragile() = default;

  EMPTY_BYTE_CONSTRUCTOR(Fragile)
  DEFAULT_SERIALIZE(ByteArray {})
};
