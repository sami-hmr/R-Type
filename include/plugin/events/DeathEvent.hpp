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
  DeathEvent(Ecs::Entity e)
      : entity(std::move(e))
  {
  }

  CHANGE_ENTITY(result.entity = map.at(entity);)

  DEFAULT_BYTE_CONSTRUCTOR(DeathEvent,
                           ([](Ecs::Entity const& e)
                            { return (DeathEvent) {e}; }),
                           parseByte<Ecs::Entity>())

  DEFAULT_SERIALIZE(type_to_byte(this->entity))

  DeathEvent(Registry& r, JsonObject const& e, std::optional<Ecs::Entity> entity)
      : entity(static_cast<Ecs::Entity>(
            get_value_copy<double>(r, e, "entity", entity).value()))
  {
  }

  Ecs::Entity entity;
};
