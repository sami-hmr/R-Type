#pragma once

#include <vector>

#include "ByteParser/ByteParser.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/Byte.hpp"
#include "plugin/HookMacros.hpp"
#include "plugin/events/EventMacros.hpp"

struct BasicMap
{
  Vector2D size;
  std::vector<std::vector<int>> data;

  BasicMap(Vector2D size, std::vector<std::vector<int>> data)
      : size(size)
      , data(std::move(data))
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(
      BasicMap,
      [](Vector2D size, std::vector<std::vector<int>> data)
      { return BasicMap(size, std::move(data)); },
      parseVector2D(),
      parseByteArray(parseByteArray(parseByte<int>())))

  DEFAULT_SERIALIZE(
      vector2DToByte(this->size),
      vector_to_byte(this->data,
                     SERIALIZE_FUNCTION<std::vector<int>>(vector_to_byte<int>,
                                                          TTB_FUNCTION<int>())))

  CHANGE_ENTITY_DEFAULT

  HOOKABLE(BasicMap, HOOK(size), HOOK(data))
};
