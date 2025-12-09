/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** HealEvent
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

struct HealEvent
{
  HealEvent(Registry::Entity t, Registry::Entity s, int a)
    : target(std::move(t))
    , source(std::move(s))
    , amount(a) {}

  DEFAULT_BYTE_CONSTRUCTOR(HealEvent,
                         ([](Registry::Entity const &t, Registry::Entity const &s, int a)
                          { return (HealEvent) {t, s, a}; }),
                         parseByte<Registry::Entity>(),
                         parseByte<Registry::Entity>(),
                         parseByte<int>())

  DEFAULT_SERIALIZE(type_to_byte(this->target), type_to_byte(this->source), type_to_byte(this->amount))

  Registry::Entity target;
  Registry::Entity source;
  int amount;
};
