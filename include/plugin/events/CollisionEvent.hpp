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

#include "ByteParser/ByteParser.hpp"
#include "ecs/Entity.hpp"
#include "ecs/Registry.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/events/EventMacros.hpp"

struct CollisionEvent
{
  Ecs::Entity a;
  Ecs::Entity b;

  CHANGE_ENTITY(result.a = map.at(a); result.b = map.at(b);)

  CollisionEvent(Ecs::Entity c, Ecs::Entity d)
      : a(c)
      , b(d)
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(CollisionEvent,
                           ([](Ecs::Entity const& c, Ecs::Entity const& d)
                            { return CollisionEvent(c, d); }),
                           parseByte<Ecs::Entity>(),
                           parseByte<Ecs::Entity>())

  DEFAULT_SERIALIZE(type_to_byte(this->a), type_to_byte(this->b))

  CollisionEvent(Registry& r,
                 JsonObject const& e,
                 std::optional<Ecs::Entity> entity)
      : a(static_cast<Ecs::Entity>(
            get_value_copy<double>(r, e, "a", entity).value()))
      , b(static_cast<Ecs::Entity>(
            get_value_copy<double>(r, e, "b", entity).value()))
  {
  }
};

struct UpdateDirection
{
  Ecs::Entity entity;
  double x_axis;
  double y_axis;

  CHANGE_ENTITY(result.entity = map.at(entity))

  UpdateDirection(Ecs::Entity e, double x, double y)
      : entity(e)
      , x_axis(x)
      , y_axis(y)
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(UpdateDirection,
                           ([](Ecs::Entity e, double x, double y)
                            { return UpdateDirection(e, x, y); }),
                           parseByte<Ecs::Entity>(),
                           parseByte<double>(),
                           parseByte<double>())

  DEFAULT_SERIALIZE(type_to_byte(entity),
                    type_to_byte(x_axis),
                    type_to_byte(y_axis))

  UpdateDirection(Registry& r,
                  JsonObject const& e,
                  std::optional<Ecs::Entity> entity)
      : entity(get_value_copy<Ecs::Entity>(r, e, "entity", entity).value())
      , x_axis(get_value_copy<double>(r, e, "x", entity).value())
      , y_axis(get_value_copy<double>(r, e, "y", entity).value())
  {
  }
};
