#pragma once

#include "ParserUtils.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"

struct Fragile
{
  Fragile() = default;

  Fragile(bool enabled)
      : enabled(enabled)
  {
  }

  EMPTY_BYTE_CONSTRUCTOR(Fragile)
  DEFAULT_SERIALIZE(ByteArray {})

  bool enabled = true;

  HOOKABLE(Fragile, HOOK(enabled))
};
