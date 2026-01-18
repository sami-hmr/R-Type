#pragma once

#include "ByteParser/ByteParser.hpp"
#include "ParserUtils.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/Byte.hpp"
#include "plugin/HookMacros.hpp"
#include "plugin/Hooks.hpp"

struct Drawable
{
  Drawable() = default;

  Drawable(bool enabled, bool stretch = false, Vector2D const& v = {0, 0})
      : enabled(enabled)
      , stretch(stretch)
      , true_size(v)
  {
  }
  DEFAULT_BYTE_CONSTRUCTOR(Drawable,
                           ([](bool e, bool s, Vector2D const& v)
                            { return Drawable(e, s, v); }),
                           parseByte<bool>(),
                           parseByte<bool>(),
                           parseVector2D())
  DEFAULT_SERIALIZE(type_to_byte<bool>(this->enabled),
                    type_to_byte<bool>(this->stretch),
                    vector2DToByte(true_size))

  CHANGE_ENTITY_DEFAULT

  bool enabled = true;
  bool stretch = false;
  Vector2D true_size;

  HOOKABLE(Drawable,
           HOOK(enabled),
           HOOK(stretch),
           HOOK(true_size),
           HOOK(true_size.y),
           HOOK(true_size.x))
};
