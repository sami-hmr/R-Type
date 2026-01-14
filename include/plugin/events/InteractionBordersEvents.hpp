#pragma once

#include "ByteParser/ByteParser.hpp"
#include "ecs/Registry.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"

template<typename Type>
struct ReachBorders
{
  Registry::Entity zone;
  Registry::Entity player;

  ReachBorders(Registry::Entity zone, Registry::Entity player)
      : zone(zone)
      , player(player)
  {
  }

  ReachBorders(Registry& r, JsonObject const& e)
      : zone(static_cast<Registry::Entity>(
            get_value_copy<double>(r, e, "zone").value()))
      , player(static_cast<Registry::Entity>(
            get_value_copy<double>(r, e, "player").value()))
  {
  }
  DEFAULT_BYTE_CONSTRUCTOR(ReachBorders,
                           ([](Registry::Entity zone, Registry::Entity player)
                            { return ReachBorders(zone, player); }),
                           parseByte<Registry::Entity>(),
                           parseByte<Registry::Entity>())

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
