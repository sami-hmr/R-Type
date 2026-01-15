#pragma once

#include "ByteParser/ByteParser.hpp"
#include "ParserUtils.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"

struct Drawable
{
  Drawable() = default;

  Drawable(bool enabled)
      : enabled(enabled)
  {
  }
  DEFAULT_BYTE_CONSTRUCTOR(Drawable,
                           ([](bool e) { return Drawable(e); }),
                           parseByte<bool>())
  DEFAULT_SERIALIZE(type_to_byte<bool>(this->enabled))

  CHANGE_ENTITY_DEFAULT

  bool enabled = true;
  Vector2D true_size;

  HOOKABLE(Drawable, HOOK(enabled), HOOK(true_size))
};
