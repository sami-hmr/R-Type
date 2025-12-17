/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** CollisionEvent
*/

#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <utility>

#include "ByteParser/ByteParser.hpp"
#include "ecs/Registry.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/events/EventMacros.hpp"

struct CollisionEvent
{
  Registry::Entity a;
  Registry::Entity b;

  CHANGE_ENTITY(result.a = map.at(a); result.b = map.at(b);)

  CollisionEvent(Registry::Entity c, Registry::Entity d)
      : a(c)
      , b(d)
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(CollisionEvent,
                           ([](Registry::Entity const& c,
                               Registry::Entity const& d)
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

struct UpdateVelocity
{
  std::size_t entity;
  double x_axis;
  double y_axis;

  CHANGE_ENTITY(result.entity = map.at(entity))

  UpdateVelocity(std::size_t e, double x, double y)
      : entity(e)
      , x_axis(x)
      , y_axis(y)
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(UpdateVelocity,
                           ([](std::size_t e, double x, double y)
                            { return UpdateVelocity(e, x, y); }),
                           parseByte<std::size_t>(),
                           parseByte<double>(),
                           parseByte<double>())

  DEFAULT_SERIALIZE(type_to_byte(entity),
                    type_to_byte(x_axis),
                    type_to_byte(y_axis))

  UpdateVelocity(Registry& r, JsonObject const& e)
      : entity(get_value_copy<int>(r, e, "entity").value())
      , x_axis(get_value_copy<double>(r, e, "x").value())
      , y_axis(get_value_copy<double>(r, e, "y").value())
  {
  }
};
