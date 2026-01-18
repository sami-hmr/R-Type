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
#include "ecs/Entity.hpp"
#include "ecs/Registry.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"

struct DeathEvent
{
  DeathEvent(Ecs::Entity e, Ecs::Entity k)
      : entity(e), killer(k)
  {
  }

  CHANGE_ENTITY(result.entity = map.at(entity), result.killer = map.at(killer);)

  DEFAULT_BYTE_CONSTRUCTOR(DeathEvent,
                           ([](Ecs::Entity const& e, Ecs::Entity const& k) { return DeathEvent(e, k); }),
                           parseByte<Ecs::Entity>(), parseByte<Ecs::Entity>())

  DEFAULT_SERIALIZE(type_to_byte(this->entity), type_to_byte(this->killer))

  DeathEvent(Registry& r,
             JsonObject const& e,
             std::optional<Ecs::Entity> entity)
      : entity(static_cast<Ecs::Entity>(
            get_value_copy<double>(r, e, "entity", entity).value())),
        killer(static_cast<Ecs::Entity>(
            get_value_copy<double>(r, e, "killer", entity).value()))
  {
  }

  Ecs::Entity entity;
  Ecs::Entity killer;
};
