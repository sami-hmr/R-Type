#pragma once

#include "ByteParser/ByteParser.hpp"
#include "ecs/Registry.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/events/EventMacros.hpp"

struct FireBullet
{
  FireBullet() = default;

  FireBullet(Registry::Entity e)
      : entity(e)
  {
  }

  CHANGE_ENTITY(result.entity = map.at(entity))

  DEFAULT_BYTE_CONSTRUCTOR(FireBullet,
                           ([](Registry::Entity e) { return FireBullet(e); }),
                           parseByte<Registry::Entity>())

  DEFAULT_SERIALIZE(type_to_byte(entity))

  FireBullet(Registry& r, JsonObject const& conf)
      : entity(*get_value_copy<int>(r, conf, "entity"))
  {
  }

  Registry::Entity entity;
};
