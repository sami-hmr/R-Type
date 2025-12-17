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
#include "plugin/Hooks.hpp"
#include "plugin/events/EventMacros.hpp"

struct SceneChangeEvent
{
  SceneChangeEvent() = default;

  SceneChangeEvent(std::string t, std::string r, bool f)
      : target_scene(std::move(t))
      , reason(std::move(r))
      , force(f)
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(
      SceneChangeEvent,
      ([](std::string const& t, std::string const& r, bool f)
       { return (SceneChangeEvent) {t, r, f}; }),
      parseByteString(),
      parseByteString(),
      parseByte<bool>())

  DEFAULT_SERIALIZE(string_to_byte(this->target_scene),
                    string_to_byte(this->reason),
                    type_to_byte(this->force))

  SceneChangeEvent(Registry& r, JsonObject const& e)
      : target_scene(get_value_copy<std::string>(r, e, "target_scene").value_or(""))
      , state(get_value_copy<std::string>(r, e, "state").value_or(""))
      , reason(get_value_copy<std::string>(r, e, "reason").value_or(""))
      , force(get_value_copy<bool>(r, e, "force").value_or(false))
  {
  }

  CHANGE_ENTITY_DEFAULT

  std::string target_scene;
  std::string state;
  std::string reason;
  bool force;
};
