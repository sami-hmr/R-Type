#pragma once

#include <vector>

#include "ByteParser/ByteParser.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/Byte.hpp"
#include "plugin/HookMacros.hpp"
#include "plugin/events/EventMacros.hpp"

struct Formation
{
  Formation() = default;

  explicit Formation(float str)
      : strength(str)
      , initial_offsets()
      , active(true)
  {
  }

  Formation(float str, std::vector<Vector2D> offsets, bool act)
      : strength(str)
      , initial_offsets(std::move(offsets))
      , active(act)
  {
  }

  HOOKABLE(Formation, HOOK(strength), HOOK(initial_offsets), HOOK(active))

  CHANGE_ENTITY_DEFAULT

  DEFAULT_BYTE_CONSTRUCTOR(
      Formation,
      ([](float str, std::vector<Vector2D> offsets, bool act)
       { return Formation {str, std::move(offsets), act}; }),
      parseByte<float>(),
      parseByteArray(parseVector2D()),
      parseByte<bool>())

  DEFAULT_SERIALIZE(
      type_to_byte(strength),
      vector_to_byte(initial_offsets,
                     SERIALIZE_FUNCTION<Vector2D>(vector2DToByte)),
      type_to_byte(active))

  float strength;
  std::vector<Vector2D> initial_offsets;
  bool active;
};
