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
  Ecs::Entity target;
  Ecs::Entity source;
  int amount;

  CHANGE_ENTITY(result.target = map.at(target); result.source = map.at(source);)

  HealEvent(Ecs::Entity t, Ecs::Entity s, int a)
      : target(t)
      , source(s)
      , amount(a)
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(HealEvent,
                           ([](Ecs::Entity const& t,
                               Ecs::Entity const& s,
                               int a) { return HealEvent(t, s, a); }),
                           parseByte<Ecs::Entity>(),
                           parseByte<Ecs::Entity>(),
                           parseByte<int>())

  DEFAULT_SERIALIZE(type_to_byte(this->target),
                    type_to_byte(this->source),
                    type_to_byte(this->amount))

  HealEvent(Registry& r, JsonObject const& e, std::optional<Ecs::Entity> entity)
      : target(static_cast<Ecs::Entity>(
            get_value_copy<double>(r, e, "entity", entity).value()))
      , source(static_cast<Ecs::Entity>(
            get_value_copy<double>(r, e, "source", entity).value()))
      , amount(get_value_copy<int>(r, e, "amount", entity).value())
  {
  }
};
