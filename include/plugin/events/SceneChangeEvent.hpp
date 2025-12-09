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
#include "plugin/Byte.hpp"
#include "ecs/Registry.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/events/EventMacros.hpp"

struct SceneChangeEvent
{
  SceneChangeEvent() = default;

  SceneChangeEvent(std::string t, std::string r)
    : target_scene(std::move(t))
    , reason(std::move(r)) {}

  DEFAULT_BYTE_CONSTRUCTOR(SceneChangeEvent,
                         ([](std::string const &t, std::string const &r)
                          { return (SceneChangeEvent) {t, r}; }),
                         parseByteString(),
                         parseByteString())

  DEFAULT_SERIALIZE(string_to_byte(this->target_scene), string_to_byte(this->reason))

  SceneChangeEvent(Registry& r, JsonObject const& e)
      : target_scene(get_value_copy<std::string>(r, e, "target_scene").value())
      , state(get_value_copy<std::string>(r, e, "state").value())
      , reason(get_value_copy<std::string>(r, e, "reason").value())
  {
  }

  CHANGE_ENTITY_DEFAULT

  std::string target_scene;
  std::string state;
  std::string reason;
};
