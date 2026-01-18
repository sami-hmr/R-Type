#pragma once

#include "ecs/Registry.hpp"
#include "plugin/Byte.hpp"
#include "plugin/events/EventMacros.hpp"

struct CliStart
{
  CliStart() = default;
  EMPTY_BYTE_CONSTRUCTOR(CliStart)
  DEFAULT_SERIALIZE(ByteArray {})

  CHANGE_ENTITY_DEFAULT

  CliStart(Registry&, JsonObject const&, std::optional<Ecs::Entity>) {}
};

struct CliStop
{
  CliStop() = default;
  EMPTY_BYTE_CONSTRUCTOR(CliStop)
  DEFAULT_SERIALIZE(ByteArray {})

  CHANGE_ENTITY_DEFAULT

  CliStop(Registry&, JsonObject const&, std::optional<Ecs::Entity>) {}
};

struct CliComp
{
  CliComp() = default;
  EMPTY_BYTE_CONSTRUCTOR(CliComp)
  DEFAULT_SERIALIZE(ByteArray {})
};
