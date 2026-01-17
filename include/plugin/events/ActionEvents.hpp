#pragma once

#include <optional>
#include <string>
#include <unordered_map>

#include "ByteParser/ByteParser.hpp"
#include "EventMacros.hpp"
#include "Json/JsonParser.hpp"
#include "ecs/Registry.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/components/ActionTrigger.hpp"
#include "plugin/events/EventMacros.hpp"

struct SpawnEntityRequestEvent
{
  std::string entity_template;
  JsonObject params;

  CHANGE_ENTITY_DEFAULT

  SpawnEntityRequestEvent(std::string templ, JsonObject p)
      : entity_template(std::move(templ))
      , params(std::move(p))
  {
  }

  SpawnEntityRequestEvent(Registry& r,
                          JsonObject const& e,
                          std::optional<Ecs::Entity> entity)
      : entity_template(
            get_value_copy<std::string>(r, e, "entity_template", entity)
                .value())
      , params(get_value_copy<JsonObject>(r, e, "params", entity)
                   .value_or(JsonObject {}))
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
  Ecs::Entity target;
  std::string reason;

  CHANGE_ENTITY(result.target = map.at(target);)

  KillEntityRequestEvent(Ecs::Entity t, std::string r)
      : target(t)
      , reason(std::move(r))
  {
  }

  KillEntityRequestEvent(Registry& r,
                         JsonObject const& e,
                         std::optional<Ecs::Entity> entity)
      : target(static_cast<Ecs::Entity>(
            get_value_copy<double>(r, e, "entity", entity).value()))
      , reason(get_value_copy<std::string>(r, e, "reason", entity).value_or(""))
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(KillEntityRequestEvent,
                           ([](Ecs::Entity e, std::string const& r)
                            { return KillEntityRequestEvent(e, r); }),
                           parseByte<Ecs::Entity>(),
                           parseByteString())

  DEFAULT_SERIALIZE(type_to_byte(this->target), string_to_byte(this->reason))
};

struct ModifyComponentRequestEvent
{
  Ecs::Entity target;
  std::string component_name;
  JsonObject modifications;

  CHANGE_ENTITY(result.target = map.at(target);)

  ModifyComponentRequestEvent(Ecs::Entity t,
                              std::string comp,
                              JsonObject mods = {})
      : target(t)
      , component_name(std::move(comp))
      , modifications(std::move(mods))
  {
  }

  ModifyComponentRequestEvent(Registry& r,
                              JsonObject const& e,
                              std::optional<Ecs::Entity> entity)
      : target(static_cast<Ecs::Entity>(
            get_value_copy<double>(r, e, "entity", entity).value()))
      , component_name(
            get_value_copy<std::string>(r, e, "component", entity).value())
      , modifications(
            get_value_copy<JsonObject>(r, e, "modifications", entity).value())
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(ModifyComponentRequestEvent,
                           ([](Ecs::Entity e, std::string const& r)
                            { return ModifyComponentRequestEvent(e, r); }),
                           parseByte<Ecs::Entity>(),
                           parseByteString())

  DEFAULT_SERIALIZE(type_to_byte(this->target),
                    string_to_byte(this->component_name))
};

struct TimerTickEvent
{
  double delta_time;
  std::chrono::steady_clock::time_point now;

  TimerTickEvent(double dt)
      : delta_time(dt)
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(TimerTickEvent,
                           ([](double dt) { return TimerTickEvent(dt); }),
                           parseByte<double>())
  CHANGE_ENTITY_DEFAULT

  TimerTickEvent(double dt, std::chrono::steady_clock::time_point n)
      : delta_time(dt)
      , now(n)
  {
  }

  TimerTickEvent(Registry& r,
                 JsonObject const& e,
                 std::optional<Ecs::Entity> entity)
      : delta_time(get_value_copy<double>(r, e, "delta_time", entity).value())
      , now(std::chrono::steady_clock::now())
  {
  }
};
