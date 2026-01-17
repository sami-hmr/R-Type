/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** HealEvent
*/

#pragma once

#include "EventMacros.hpp"
#include "ecs/Registry.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/events/EventMacros.hpp"

struct HealEvent
{
  Registry::Entity target;
  int amount;

  CHANGE_ENTITY(result.target = map.at(target);)

  HealEvent(Registry::Entity t, int a)
      : target(t)
      , amount(a)
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(HealEvent,
                           ([](Registry::Entity const& t,
                               int a) { return HealEvent(t, a); }),
                           parseByte<Registry::Entity>(),
                           parseByte<int>())

  DEFAULT_SERIALIZE(type_to_byte(this->target),
                    type_to_byte(this->amount))

  HealEvent(Registry& r, JsonObject const& e)
      : target(static_cast<Registry::Entity>(
            get_value_copy<double>(r, e, "entity").value()))
      , amount(get_value_copy<int>(r, e, "amount").value())
  {
  }
};
