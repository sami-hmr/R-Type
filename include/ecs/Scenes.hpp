#pragma once

#include <cstdint>
#include <string>

#include "ByteParser/ByteParser.hpp"
#include "TwoWayMap.hpp"
#include "plugin/Byte.hpp"
#include "plugin/HookMacros.hpp"
#include "plugin/events/EventMacros.hpp"

enum class SceneState : std::uint8_t
{
  ACTIVE,
  MAIN,
  DISABLED
};

static const TwoWayMap<SceneState, std::string> SCENE_STATE_STR = {
    {SceneState::ACTIVE, "active"},
    {SceneState::MAIN, "main"},
    {SceneState::DISABLED, "disabled"},
};

struct Scene
{
  Scene() = default;

  Scene(std::string scene_name, SceneState state)
      : scene_name(std::move(scene_name))
      , state(state)
  {
  }


  DEFAULT_BYTE_CONSTRUCTOR(
      Scene,
      ([](std::vector<char> name, SceneState state)
       { return Scene(std::string(name.begin(), name.end()), state); }),
      parseByteArray(parseAnyChar()),
      parseByte<SceneState>())
  DEFAULT_SERIALIZE(string_to_byte(this->scene_name),
                    type_to_byte((std::uint8_t)this->state))

  CHANGE_ENTITY_DEFAULT

  HOOKABLE(Scene, HOOK(scene_name), HOOK(state))

  std::string scene_name;
  SceneState state;
};
