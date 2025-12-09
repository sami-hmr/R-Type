/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** CollisionEvent
*/

#pragma once

#include <optional>
#include <string>
#include <utility>

#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"
#include "ecs/Registry.hpp"
#include "ByteParser/ByteParser.hpp"
#include "plugin/events/EventMacros.hpp"

struct CollisionEvent
{
  Registry::Entity a;
  Registry::Entity b;

  CHANGE_ENTITY(result.a = map.at_second(a); result.b = map.at_second(b);)

  CollisionEvent(Registry::Entity c, Registry::Entity d)
    : a(c)
    , b(d) {}

  DEFAULT_BYTE_CONSTRUCTOR(CollisionEvent,
                         ([](Registry::Entity const &c, Registry::Entity const &d)
                          { return (CollisionEvent) {c, d}; }),
                         parseByte<Registry::Entity>(),
                         parseByte<Registry::Entity>())

  DEFAULT_SERIALIZE(type_to_byte(this->a), type_to_byte(this->b))

  CollisionEvent(Registry& r, JsonObject const& e)
      : a(static_cast<Registry::Entity>(
            get_value_copy<double>(r, e, "a").value()))
      , b(static_cast<Registry::Entity>(
            get_value_copy<double>(r, e, "b").value()))
  {
  }
};
