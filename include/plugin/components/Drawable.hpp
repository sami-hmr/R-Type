#pragma once

#include "ParserUtils.hpp"
#include "plugin/Byte.hpp"

struct Drawable
{
  Drawable() = default;

  Drawable(bool enabled)
      : enabled(enabled)
  {
  }
  EMPTY_BYTE_CONSTRUCTOR(Drawable)
  DEFAULT_SERIALIZE(ByteArray {})

  bool enabled = true;
};
