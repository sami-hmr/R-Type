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
#include "plugin/events/EventMacros.hpp"

struct CliStart
{
<<<<<<< Updated upstream
    CliStart() = default;
    EMPTY_BYTE_CONSTRUCTOR(CliStart)
    DEFAULT_SERIALIZE(ByteArray{})
=======
  CliStart() = default;

  CHANGE_ENTITY_DEFAULT

  CliStart(Registry&, JsonObject const&) {}
>>>>>>> Stashed changes
};

struct CliStop
{
<<<<<<< Updated upstream
    CliStop() = default;
    EMPTY_BYTE_CONSTRUCTOR(CliStop)
    DEFAULT_SERIALIZE(ByteArray{})
=======
  CliStop() = default;

  CHANGE_ENTITY_DEFAULT

  CliStop(Registry&, JsonObject const&) {}
>>>>>>> Stashed changes
};

struct CliComp
{
  CliComp() = default;
  EMPTY_BYTE_CONSTRUCTOR(CliComp)
  DEFAULT_SERIALIZE(ByteArray {})
};
