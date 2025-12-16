/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** HealEvent
*/

#pragma once

#include "EventMacros.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"
#include "ecs/Registry.hpp"
#include "plugin/events/EventMacros.hpp"

struct HealEvent
{
  Registry::Entity target;
  Registry::Entity source;
  int amount;

  CHANGE_ENTITY(result.target = map.at(target);
                result.source = map.at(source);)

  HealEvent(Registry::Entity t, Registry::Entity s, int a)
    : target(t)
    , source(s)
    , amount(a) {}

  DEFAULT_BYTE_CONSTRUCTOR(HealEvent,
                         ([](Registry::Entity const &t, Registry::Entity const &s, int a)
                          { return (HealEvent) {t, s, a}; }),
                         parseByte<Registry::Entity>(),
                         parseByte<Registry::Entity>(),
                         parseByte<int>())

  DEFAULT_SERIALIZE(type_to_byte(this->target), type_to_byte(this->source), type_to_byte(this->amount))

  HealEvent(Registry& r, JsonObject const& e)
      : target(static_cast<Registry::Entity>(
            get_value_copy<double>(r, e, "entity").value()))
      , source(static_cast<Registry::Entity>(
            get_value_copy<double>(r, e, "source").value()))
      , amount(get_value_copy<int>(r, e, "amount").value())
  {
  }
};
