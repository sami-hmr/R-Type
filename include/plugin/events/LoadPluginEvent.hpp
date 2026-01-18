#pragma once

#include <optional>
#include <string>
#include <unordered_map>
#include <utility>

#include "ByteParser/ByteParser.hpp"
#include "EventMacros.hpp"
#include "Json/JsonParser.hpp"
#include "ecs/Registry.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/components/ActionTrigger.hpp"
#include "plugin/events/EventMacros.hpp"

struct LoadPluginEvent
{
  std::string path;
  JsonObject params;

  LoadPluginEvent(std::string path, JsonObject p = {})
      : path(std::move(path))
      , params(std::move(p))
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(
      LoadPluginEvent,
      ([](std::string p, JsonObject pm)
       { return LoadPluginEvent(std::move(p), std::move(pm)); }),
      parseByteString(),
      parseJsonObject())

  DEFAULT_SERIALIZE(string_to_byte(path), json_object_to_byte(params))

  CHANGE_ENTITY_DEFAULT

  LoadPluginEvent(Registry& r,
                  JsonObject const& e,
                  std::optional<Ecs::Entity> entity)
      : path(get_value_copy<std::string>(r, e, "path", entity).value())
      , params(get_value_copy<JsonObject>(r, e, "params", entity).value())
  {
  }
};

struct LoadConfigEvent
{
  std::string path;

  LoadConfigEvent(std::string path)
      : path(std::move(path))
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(LoadConfigEvent,
                           ([](std::string p)
                            { return LoadConfigEvent(std::move(p)); }),
                           parseByteString())

  DEFAULT_SERIALIZE(string_to_byte(path))

  CHANGE_ENTITY_DEFAULT

  LoadConfigEvent(Registry& r,
                  JsonObject const& e,
                  std::optional<Ecs::Entity> entity)
      : path(get_value_copy<std::string>(r, e, "path", entity).value())
  {
  }
};
