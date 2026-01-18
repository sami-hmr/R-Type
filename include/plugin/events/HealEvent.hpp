/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** HealEvent
*/

#pragma once

#include "EventMacros.hpp"
#include "ecs/Entity.hpp"
#include "ecs/Registry.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/events/EventMacros.hpp"

struct HealEvent
{
  Ecs::Entity target;
  int amount;

  CHANGE_ENTITY(result.target = map.at(target);)

  HealEvent(Ecs::Entity t, int a)
      : target(t)
      , amount(a)
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(HealEvent,
                           ([](Ecs::Entity const& t, int a)
                            { return HealEvent(t, a); }),
                           parseByte<Ecs::Entity>(),
                           parseByte<int>())

  DEFAULT_SERIALIZE(type_to_byte(this->target), type_to_byte(this->amount))

  HealEvent(Registry& r, JsonObject const& e, std::optional<Ecs::Entity> entity)
      : target(static_cast<Ecs::Entity>(
            get_value_copy<Ecs::Entity>(r, e, "entity", entity).value()))
      , amount(get_value_copy<int>(r, e, "amount", entity).value())
  {
  }
};
