#pragma once

#include "ParserUtils.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"

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

  HOOKABLE(Drawable, HOOK(enabled))
};
