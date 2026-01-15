#pragma once

#include "ByteParser/ByteParser.hpp"
#include "ParserUtils.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"

struct Drawable
{
  Drawable() = default;

  Drawable(bool enabled, Vector2D const& v = {0, 0})
      : enabled(enabled)
      , true_size(v)
  {
  }
  DEFAULT_BYTE_CONSTRUCTOR(Drawable,
                           ([](bool e, Vector2D const& v)
                            { return Drawable(e, v); }),
                           parseByte<bool>(),
                           parseVector2D())
  DEFAULT_SERIALIZE(type_to_byte<bool>(this->enabled),
                    vector2DToByte(true_size))

  CHANGE_ENTITY_DEFAULT

  bool enabled = true;
  Vector2D true_size;

  HOOKABLE(Drawable, HOOK(enabled), HOOK(true_size))
};
