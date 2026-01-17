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
  DeathEvent(Registry::Entity e, Registry::Entity k)
      : entity(e)
      , killer(k)
  {
  }

  CHANGE_ENTITY(result.entity = map.at(entity), result.killer = map.at(killer);)

  DEFAULT_BYTE_CONSTRUCTOR(DeathEvent,
                           ([](Registry::Entity const& e,
                               Registry::Entity const& k)
                            { return DeathEvent {e, k}; }),
                           parseByte<Registry::Entity>(),
                           parseByte<Registry::Entity>())

  DEFAULT_SERIALIZE(type_to_byte(this->entity), type_to_byte(this->killer))

  DeathEvent(Registry& r, JsonObject const& e)
      : entity(static_cast<Registry::Entity>(
            get_value_copy<double>(r, e, "entity").value()))
      , killer(static_cast<Registry::Entity>(
            get_value_copy<double>(r, e, "killer").value()))
  {
  }

  Registry::Entity entity;
  Registry::Entity killer;
};
