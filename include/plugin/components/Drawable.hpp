#pragma once

#include "plugin/Byte.hpp"
#include "ParserUtils.hpp"

struct Drawable
{
  Drawable() = default;
  EMPTY_BYTE_CONSTRUCTOR(Drawable)
  DEFAULT_SERIALIZE(ByteArray{})
};
