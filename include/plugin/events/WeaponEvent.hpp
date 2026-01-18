#pragma once

#include "ByteParser/ByteParser.hpp"
#include "ecs/Registry.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/events/EventMacros.hpp"

struct FireBullet
{
  FireBullet() = default;

  FireBullet(Ecs::Entity e)
      : entity(e)
  {
  }

  CHANGE_ENTITY(result.entity = map.at(entity))

  DEFAULT_BYTE_CONSTRUCTOR(FireBullet,
                           ([](Ecs::Entity e) { return FireBullet(e); }),
                           parseByte<Ecs::Entity>())

  DEFAULT_SERIALIZE(type_to_byte(entity))

  FireBullet(Registry& r,
             JsonObject const& conf,
             std::optional<Ecs::Entity> entity)
      : entity(*get_value_copy<int>(r, conf, "entity", entity))
  {
  }

  Ecs::Entity entity;
};

struct StartChargeWeapon
{
  StartChargeWeapon() = default;

  StartChargeWeapon(Ecs::Entity e)
      : entity(e)
  {
  }

  CHANGE_ENTITY(result.entity = map.at(entity))

  DEFAULT_BYTE_CONSTRUCTOR(StartChargeWeapon,
                           ([](Ecs::Entity e) { return StartChargeWeapon(e); }),
                           parseByte<Ecs::Entity>())

  DEFAULT_SERIALIZE(type_to_byte(entity))

  StartChargeWeapon(Registry& r,
                    JsonObject const& conf,
                    std::optional<Ecs::Entity> entity)
      : entity(*get_value_copy<int>(r, conf, "entity", entity))
  {
  }

  Ecs::Entity entity;
};

struct ReleaseChargeWeapon
{
  ReleaseChargeWeapon() = default;

  ReleaseChargeWeapon(Ecs::Entity e)
      : entity(e)
  {
  }

  CHANGE_ENTITY(result.entity = map.at(entity))

  DEFAULT_BYTE_CONSTRUCTOR(ReleaseChargeWeapon,
                           ([](Ecs::Entity e)
                            { return ReleaseChargeWeapon(e); }),
                           parseByte<Ecs::Entity>())

  DEFAULT_SERIALIZE(type_to_byte(entity))

  ReleaseChargeWeapon(Registry& r,
                      JsonObject const& conf,
                      std::optional<Ecs::Entity> entity)
      : entity(*get_value_copy<int>(r, conf, "entity", entity))
  {
  }

  Ecs::Entity entity;
};
