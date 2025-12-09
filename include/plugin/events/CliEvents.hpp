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

struct CliStart
{
    CliStart() = default;
    EMPTY_BYTE_CONSTRUCTOR(CliStart)
    DEFAULT_SERIALIZE(ByteArray{})
};

struct CliStop
{
    CliStop() = default;
    EMPTY_BYTE_CONSTRUCTOR(CliStop)
    DEFAULT_SERIALIZE(ByteArray{})
};

struct CliComp
{
  CliComp() = default;
  EMPTY_BYTE_CONSTRUCTOR(CliComp)
  DEFAULT_SERIALIZE(ByteArray {})
};
