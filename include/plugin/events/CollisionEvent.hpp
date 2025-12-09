/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** CollisionEvent
*/

#pragma once

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <utility>

#include "ByteParser/ByteParser.hpp"
#include "ParserUtils.hpp"
#include "ecs/Registry.hpp"
#include "plugin/Byte.hpp"

struct CollisionEvent
{
  CollisionEvent(Registry::Entity c, Registry::Entity d)
    : a(std::move(c))
    , b(std::move(d)) {}

  DEFAULT_BYTE_CONSTRUCTOR(CollisionEvent,
                         ([](Registry::Entity const &c, Registry::Entity const &d)
                          { return (CollisionEvent) {c, d}; }),
                         parseByte<Registry::Entity>(),
                         parseByte<Registry::Entity>())

  DEFAULT_SERIALIZE(type_to_byte(this->a), type_to_byte(this->b))

  Registry::Entity a;
  Registry::Entity b;
};
