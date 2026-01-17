/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** DamageEvent
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

struct DamageEvent
{
  DamageEvent(Ecs::Entity t, Ecs::Entity s, int a)
      : target(std::move(t))
      , source(std::move(s))
      , amount(a)
  {
  }

  CHANGE_ENTITY(result.target = map.at(target); result.source = map.at(source);)

  DEFAULT_BYTE_CONSTRUCTOR(DamageEvent,
                           ([](Ecs::Entity const& t,
                               Ecs::Entity const& s,
                               int a) { return (DamageEvent) {t, s, a}; }),
                           parseByte<Ecs::Entity>(),
                           parseByte<Ecs::Entity>(),
                           parseByte<int>())

  DEFAULT_SERIALIZE(type_to_byte(this->target),
                    type_to_byte(this->source),
                    type_to_byte(this->amount))

  DamageEvent(Registry& r, JsonObject const& e, std::optional<Ecs::Entity> entity)
      : target(static_cast<Ecs::Entity>(
            get_value_copy<double>(r, e, "entity", entity).value()))
      , source(static_cast<Ecs::Entity>(
            get_value_copy<double>(r, e, "source", entity).value()))
      , amount(get_value_copy<int>(r, e, "amount", entity).value())
  {
  }

  Ecs::Entity target;
  Ecs::Entity source;
  int amount;
};
