#pragma once

#include <cstddef>

#include "ByteParser/ByteParser.hpp"
#include "Json/JsonParser.hpp"
#include "ecs/Registry.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/Byte.hpp"
#include "plugin/HookMacros.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/events/EventMacros.hpp"

struct WaveTag
{
  WaveTag() = default;

  WaveTag(std::size_t id)
      : wave_id(id)
      , formation_offset(0, 0)
  {
  }

  WaveTag(std::size_t id, Vector2D offset)
      : wave_id(id)
      , formation_offset(offset)
  {
  }

  HOOKABLE(WaveTag, HOOK(wave_id), HOOK(formation_offset))

  CHANGE_ENTITY_DEFAULT

  DEFAULT_BYTE_CONSTRUCTOR(WaveTag,
                           ([](std::size_t id, Vector2D offset)
                            { return WaveTag(id, offset); }),
                           parseByte<std::size_t>(),
                           parseVector2D())

  DEFAULT_SERIALIZE(type_to_byte(wave_id), vector2DToByte(formation_offset))

  WaveTag(Registry& r, JsonObject const& obj)
      : wave_id(get_value_copy<std::size_t>(r, obj, "wave_id").value_or(0))
      , formation_offset(get_value_copy<Vector2D>(r, obj, "formation_offset")
                             .value_or(Vector2D(0, 0)))
  {
  }

  std::size_t wave_id = 0;
  Vector2D formation_offset;
};
