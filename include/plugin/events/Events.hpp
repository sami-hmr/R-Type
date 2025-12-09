#pragma once

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "ByteParser/ByteParser.hpp"
#include "ParserUtils.hpp"
#include "ecs/Registry.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/components/ActionTrigger.hpp"
#include "plugin/components/InteractionZone.hpp"

struct SceneChangeEvent
{
  std::string target_scene;
  std::string state;
  std::string reason;

  SceneChangeEvent(Registry& r, JsonObject const& e)
      : target_scene(get_value_copy<std::string>(r, e, "target_scene").value())
      , state(get_value_copy<std::string>(r, e, "state").value())
      , reason(get_value_copy<std::string>(r, e, "reason").value())
  {
  }

  SceneChangeEvent(std::string t, std::string s, std::string r)
      : target_scene(std::move(t))
      , state(std::move(s))
      , reason(std::move(r))
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(
      SceneChangeEvent,
      ([](std::string t, std::string s, std::string r)
       { return SceneChangeEvent(std::move(t), std::move(s), std::move(r)); }),
      parseByteString(),
      parseByteString(),
      parseByteString())
};
