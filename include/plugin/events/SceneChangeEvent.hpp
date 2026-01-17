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
#include "ecs/Scenes.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/events/EventMacros.hpp"

struct SceneChangeEvent
{
  SceneChangeEvent() = default;

  SceneChangeEvent(std::string t, std::string r, bool f, bool main = false)
      : target_scene(std::move(t))
      , reason(std::move(r))
      , force(f)
      , main(main)
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(
      SceneChangeEvent,
      ([](std::string const& t, std::string const& r, bool f, bool m = false)
       { return SceneChangeEvent(t, r, f, m); }),
      parseByteString(),
      parseByteString(),
      parseByte<bool>(),
      parseByte<bool>())

  DEFAULT_SERIALIZE(string_to_byte(this->target_scene),
                    string_to_byte(this->reason),
                    type_to_byte(this->force),
                    type_to_byte(main))

  SceneChangeEvent(Registry& r,
                   JsonObject const& e,
                   std::optional<Ecs::Entity> entity)
      : target_scene(get_value_copy<std::string>(r, e, "target_scene", entity)
                         .value_or(""))
      , reason(get_value_copy<std::string>(r, e, "reason", entity).value_or(""))
      , force(get_value_copy<bool>(r, e, "force", entity).value_or(false))
      , main(get_value_copy<bool>(r, e, "main", entity).value_or(false))
  {
  }

  CHANGE_ENTITY_DEFAULT

  std::string target_scene;
  std::string reason;
  bool force;
  bool main;
};

struct DisableSceneEvent
{
  DisableSceneEvent() = default;

  DisableSceneEvent(std::string t)
      : target_scene(std::move(t))
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(DisableSceneEvent,
                           ([](std::string const& t)
                            { return DisableSceneEvent(t); }),
                           parseByteString())

  DEFAULT_SERIALIZE(string_to_byte(this->target_scene))

  DisableSceneEvent(Registry& r,
                    JsonObject const& e,
                    std::optional<Ecs::Entity> entity)
      : target_scene(get_value_copy<std::string>(r, e, "target_scene", entity)
                         .value_or(""))
  {
  }

  CHANGE_ENTITY_DEFAULT

  std::string target_scene;
};
