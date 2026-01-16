/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** DamageEvent
*/

#pragma once

#include <optional>
#include <string>

#include "ByteParser/ByteParser.hpp"
#include "EventMacros.hpp"
#include "ecs/Registry.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"

struct DamageEvent
{
  DamageEvent(Registry::Entity t, Registry::Entity s, int a)
      : target(t)
      , source(s)
      , amount(a)
  {
  }

  CHANGE_ENTITY(result.target = map.at(target); result.source = map.at(source);)

  DEFAULT_BYTE_CONSTRUCTOR(DamageEvent,
                           ([](Registry::Entity const& t,
                               Registry::Entity const& s,
                               int a) { return DamageEvent(t, s, a); }),
                           parseByte<Registry::Entity>(),
                           parseByte<Registry::Entity>(),
                           parseByte<int>())

  DEFAULT_SERIALIZE(type_to_byte(this->target),
                    type_to_byte(this->source),
                    type_to_byte(this->amount))

  DamageEvent(Registry& r, JsonObject const& e)
      : target(static_cast<Registry::Entity>(
            get_value_copy<double>(r, e, "entity").value()))
      , source(static_cast<Registry::Entity>(
            get_value_copy<double>(r, e, "source").value()))
      , amount(get_value_copy<int>(r, e, "amount").value())
  {
  }

  Registry::Entity target;
  Registry::Entity source;
  int amount;
};
