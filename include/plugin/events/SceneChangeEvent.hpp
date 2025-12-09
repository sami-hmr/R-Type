/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** SceneChangeEvent
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

struct SceneChangeEvent
{
  SceneChangeEvent(std::string t, std::string r)
    : target_scene(std::move(t))
    , reason(std::move(r)) {}

  DEFAULT_BYTE_CONSTRUCTOR(SceneChangeEvent,
                         ([](std::string const &t, std::string const &r)
                          { return (SceneChangeEvent) {t, r}; }),
                         parseByteString(),
                         parseByteString())

  DEFAULT_SERIALIZE(string_to_byte(this->target_scene), string_to_byte(this->reason))

  std::string target_scene;
  std::string reason;
};
