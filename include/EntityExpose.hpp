#pragma once

#include <cstddef>
#include <string>
#include <utility>

#include "BaseTypes.hpp"
#include "ByteParser/ByteParser.hpp"
#include "ParserUtils.hpp"
#include "plugin/Byte.hpp"
#include "plugin/HookMacros.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/events/EventMacros.hpp"

class IdStorage
{
public:
  IdStorage()
      : id_s(0)
  {
  }

  IdStorage(std::size_t id, std::string ctx)
      : id_s(id)
      , context(std::move(ctx))
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(IdStorage,
                           ([](std::size_t id, std::string ctx)
                            { return (IdStorage) {id, std::move(ctx)}; }),
                           parseByte<std::size_t>(),
                           parseByteString())

  DEFAULT_SERIALIZE(type_to_byte(this->id_s), string_to_byte(this->context))

  CHANGE_ENTITY(result.id_s = map.at(id_s))

  HOOKABLE(IdStorage, HOOK_CUSTOM(id, id_s), HOOK(context))

  std::size_t id_s;
  std::string context;
};
