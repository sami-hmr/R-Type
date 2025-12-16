#pragma once

#include "ecs/Registry.hpp"
#include "plugin/Byte.hpp"
#include "plugin/events/EventMacros.hpp"

struct FireBullet
{
  FireBullet() = default;

  CHANGE_ENTITY_DEFAULT

  EMPTY_BYTE_CONSTRUCTOR(FireBullet)

  DEFAULT_SERIALIZE(ByteArray{})

  FireBullet(Registry&, JsonObject const&) {}
};
