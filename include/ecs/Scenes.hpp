#pragma once

#include <cstdint>
#include <string>

#include "TwoWayMap.hpp"
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
  std::string scene_name;
  SceneState state;
};
