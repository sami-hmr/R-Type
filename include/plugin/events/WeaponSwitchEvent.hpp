#pragma once

#include <optional>
#include <string>
#include <unordered_map>
#include <utility>

#include "BaseTypes.hpp"
#include "ByteParser/ByteParser.hpp"
#include "EventMacros.hpp"
#include "Json/JsonParser.hpp"
#include "ecs/Entity.hpp"
#include "ecs/Registry.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/components/ActionTrigger.hpp"
#include "plugin/events/EventMacros.hpp"

struct WeaponSwitchEvent
{
  Ecs::Entity entity;
  std::string weapon_type;
  JsonObject params;

  CHANGE_ENTITY(result.entity = map.at(entity))

  WeaponSwitchEvent(Ecs::Entity entity, std::string weapon_type, JsonObject p)
      : entity(entity)
      , weapon_type(std::move(weapon_type))
      , params(std::move(p))
  {
  }

  WeaponSwitchEvent(Registry& r,
                    JsonObject const& e,
                    std::optional<Ecs::Entity> entity)
      : entity(get_value_copy<Ecs::Entity>(r, e, "entity", entity).value())
      , weapon_type(get_value_copy<std::string>(r, e, "weapon_type", entity)
                        .value_or(""))
      , params(get_value_copy<JsonObject>(r, e, "params", entity)
                   .value_or(JsonObject {}))
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(
      WeaponSwitchEvent,
      ([](Ecs::Entity e, std::string weapon_type, JsonObject o)
       { return WeaponSwitchEvent(e, std::move(weapon_type), std::move(o)); }),
      parseByte<Ecs::Entity>(),
      parseByteString(),
      parseByteJsonObject());

  DEFAULT_SERIALIZE(type_to_byte(entity),
                    string_to_byte(weapon_type),
                    json_object_to_byte(params))
};
