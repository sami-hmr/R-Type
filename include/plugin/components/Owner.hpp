#pragma once

#include "ByteParser/ByteParser.hpp"
#include "ParserUtils.hpp"
#include "plugin/Byte.hpp"

struct Owner
{
  Owner() = default;

  Owner(std::size_t owner)
      : entity_id(owner)
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(Owner,
                           ([](std::size_t entity_id)
                            { return (Owner) {entity_id}; }),
                           parseByte<int>())
  DEFAULT_SERIALIZE(type_to_byte(this->entity_id))

  std::size_t entity_id;
};
