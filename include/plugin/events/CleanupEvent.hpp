/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** CleanupEvent
*/

#pragma once

#include <optional>
#include <string>
#include <utility>

#include "ByteParser/ByteParser.hpp"
#include "ecs/Registry.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/events/EventMacros.hpp"

struct CleanupEvent
{
  std::string trigger;

  DEFAULT_BYTE_CONSTRUCTOR(CleanupEvent,
                           ([](std::string const& t)
                            { return (CleanupEvent) {t}; }),
                           parseByteString())

  DEFAULT_SERIALIZE(string_to_byte(this->trigger))

  CHANGE_ENTITY_DEFAULT

  CleanupEvent(std::string trigger)
      : trigger(std::move(trigger))
  {
  }

  CleanupEvent(Registry& r, JsonObject const& e, std::optional<Ecs::Entity> entity)
      : trigger(get_value_copy<std::string>(r, e, "trigger", entity).value())
  {
  }
};
