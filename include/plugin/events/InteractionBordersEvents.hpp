#pragma once

#include "ByteParser/ByteParser.hpp"
#include "ecs/Registry.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"

template<typename Type>
struct ReachBorders
{
  Ecs::Entity zone;
  Ecs::Entity player;

  ReachBorders(Ecs::Entity zone, Ecs::Entity player)
      : zone(zone)
      , player(player)
  {
  }

  ReachBorders(Registry& r,
               JsonObject const& e,
               std::optional<Ecs::Entity> entity)
      : zone(static_cast<Ecs::Entity>(
            get_value_copy<double>(r, e, "zone", entity).value()))
      , player(static_cast<Ecs::Entity>(
            get_value_copy<double>(r, e, "player", entity).value()))
  {
  }
  DEFAULT_BYTE_CONSTRUCTOR(ReachBorders,
                           ([](Ecs::Entity zone, Ecs::Entity player)
                            { return ReachBorders(zone, player); }),
                           parseByte<Ecs::Entity>(),
                           parseByte<Ecs::Entity>())

  DEFAULT_SERIALIZE(type_to_byte(zone), type_to_byte(player))
};

struct LeftZoneEvt
{
};

struct EnteredZoneEvt
{
};

using LeftZone = ReachBorders<LeftZoneEvt>;
using EnteredZone = ReachBorders<EnteredZoneEvt>;
