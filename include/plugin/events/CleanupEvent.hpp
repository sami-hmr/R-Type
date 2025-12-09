/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** CleanupEvent
*/

#pragma once

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <utility>

#include "ByteParser/ByteParser.hpp"
#include "ParserUtils.hpp"
#include "plugin/Byte.hpp"
#include "ecs/Registry.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/events/CleanupEvent.hpp"

struct CleanupEvent
{
  CleanupEvent(std::string t)
    : trigger(std::move(t)) {}

  DEFAULT_BYTE_CONSTRUCTOR(CleanupEvent,
                         ([](std::string const &t)
                          { return (CleanupEvent) {t}; }),
                         parseByteString())

  DEFAULT_SERIALIZE(string_to_byte(this->trigger))

  CleanupEvent(Registry& r, JsonObject const& e)
      : trigger(get_value_copy<std::string>(r, e, "trigger").value())
  {}

  std::string trigger;
};
