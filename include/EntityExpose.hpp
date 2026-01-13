#pragma once

#include <cstddef>

#include "ParserUtils.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/events/EventMacros.hpp"

class IdStorage
{
public:
  IdStorage()
      : id_s(0)
  {
  }

  IdStorage(std::size_t id)
      : id_s(id)
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(IdStorage,
                           ([](std::size_t id) { return (IdStorage) {id}; }),
                           parseByte<std::size_t>())

  DEFAULT_SERIALIZE(type_to_byte(this->id_s))

  CHANGE_ENTITY(result.id_s = map.at(id_s))

  HOOKABLE(IdStorage, HOOK_CUSTOM(id, id_s))

  std::size_t id_s;
};
