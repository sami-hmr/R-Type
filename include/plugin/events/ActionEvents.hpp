#pragma once

#include <optional>
#include <string>
#include <unordered_map>

#include "ByteParser/ByteParser.hpp"
#include "Json/JsonParser.hpp"
#include "ecs/Registry.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/components/ActionTrigger.hpp"

struct SpawnEntityRequestEvent
{
  std::string entity_template;
  JsonObject params;

  SpawnEntityRequestEvent(std::string templ, JsonObject p)
      : entity_template(std::move(templ))
      , params(std::move(p))
  {
  }

  SpawnEntityRequestEvent(Registry& r, JsonObject const& e)
      : entity_template(
            get_value_copy<std::string>(r, e, "entity_template").value())
      , params(
            get_value_copy<JsonObject>(r, e, "params").value_or(JsonObject {}))
  {
  }

  SpawnEntityRequestEvent(std::string e)
      : entity_template(std::move(e))
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(
      SpawnEntityRequestEvent,
      ([](std::string e /*, JsonObject o*/)
       { return SpawnEntityRequestEvent(std::move(e) /*, o */); }),
      parseByteString());

  DEFAULT_SERIALIZE(string_to_byte(
      this->entity_template) /*, json_object_to_byte(this->params) */)
};

struct KillEntityRequestEvent
{
  Registry::Entity target;
  std::string reason;

  KillEntityRequestEvent(Registry::Entity t, std::string r)
      : target(t)
      , reason(std::move(r))
  {
  }

  KillEntityRequestEvent(Registry& r, JsonObject const& e)
      : target(static_cast<Registry::Entity>(
            get_value_copy<double>(r, e, "target").value()))
      , reason(get_value_copy<std::string>(r, e, "reason").value_or(""))
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(KillEntityRequestEvent,
                           ([](Registry::Entity e, std::string const& r)
                            { return KillEntityRequestEvent(e, r); }),
                           parseByte<Registry::Entity>(),
                           parseByteString())

  DEFAULT_SERIALIZE(type_to_byte(this->target), string_to_byte(this->reason))
};

struct ModifyComponentRequestEvent
{
  Registry::Entity target;
  std::string component_name;
  JsonObject modifications;

  ModifyComponentRequestEvent(Registry::Entity t,
                              std::string comp,
                              JsonObject mods = {})
      : target(t)
      , component_name(std::move(comp))
      , modifications(std::move(mods))
  {
  }

  ModifyComponentRequestEvent(Registry& r, JsonObject const& e)
      : target(static_cast<Registry::Entity>(
            get_value_copy<double>(r, e, "target").value()))
      , component_name(get_value_copy<std::string>(r, e, "component").value())
      , modifications(get_value_copy<JsonObject>(r, e, "modifications").value())
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(ModifyComponentRequestEvent,
                           ([](Registry::Entity e, std::string const& r)
                            { return ModifyComponentRequestEvent(e, r); }),
                           parseByte<Registry::Entity>(),
                           parseByteString())

  DEFAULT_SERIALIZE(type_to_byte(this->target), string_to_byte(this->component_name))
};

struct TimerTickEvent
{
  double delta_time;
  std::chrono::steady_clock::time_point now;

  TimerTickEvent(double dt): delta_time(dt) {}

  DEFAULT_BYTE_CONSTRUCTOR(TimerTickEvent, ([](double dt){return TimerTickEvent(dt);}), parseByte<double>())
};
