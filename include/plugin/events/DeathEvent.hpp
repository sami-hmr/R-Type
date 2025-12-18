/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** DeathEvent
*/

#pragma once

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <utility>

#include "ByteParser/ByteParser.hpp"
#include "EventMacros.hpp"
#include "ParserUtils.hpp"
#include "ecs/Registry.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"

struct DeathEvent
{
  DeathEvent(Registry::Entity e)
      : entity(std::move(e))
  {
  }

  CHANGE_ENTITY(result.entity = map.at(entity);)

  DEFAULT_BYTE_CONSTRUCTOR(DeathEvent,
                           ([](Registry::Entity const& e)
                            { return (DeathEvent) {e}; }),
                           parseByte<Registry::Entity>())

  DEFAULT_SERIALIZE(type_to_byte(this->entity))

  DeathEvent(Registry& r, JsonObject const& e)
      : entity(static_cast<Registry::Entity>(
            get_value_copy<double>(r, e, "entity").value()))
  {
  }

  Registry::Entity entity;
};
