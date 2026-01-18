/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Shutdown
*/

#pragma once

#include <string>

#include "ByteParser/ByteParser.hpp"
#include "EventMacros.hpp"
#include "ParserUtils.hpp"
#include "ecs/Registry.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"

struct ShutdownEvent
{
  std::string reason;
  int exit_code = 0;

  DEFAULT_BYTE_CONSTRUCTOR(ShutdownEvent,
                           ([](std::string const& r, int e)
                            { return ShutdownEvent {r, e}; }),
                           parseByteString(),
                           parseByte<int>())

  DEFAULT_SERIALIZE(string_to_byte(this->reason), type_to_byte(this->exit_code))

  CHANGE_ENTITY_DEFAULT

  ShutdownEvent(std::string reason, int exit_code)
      : reason(std::move(reason))
      , exit_code(exit_code)
  {
  }

  ShutdownEvent(Registry& r,
                JsonObject const& e,
                std::optional<Ecs::Entity> entity)
      : reason(get_value_copy<std::string>(r, e, "reason", entity).value())
      , exit_code(get_value_copy<int>(r, e, "exit_code", entity).value())
  {
  }
};
