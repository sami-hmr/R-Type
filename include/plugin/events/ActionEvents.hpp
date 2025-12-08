#pragma once

#include <optional>
#include <string>

#include "ecs/Registery.hpp"
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

  SpawnEntityRequestEvent(Registery& r, JsonObject const& e)
      : entity_template(
            get_value_copy<std::string>(r, e, "entity_template").value())
      , params(
            get_value_copy<JsonObject>(r, e, "params").value_or(JsonObject {}))
  {
  }
};

struct KillEntityRequestEvent
{
  Registery::Entity target;
  std::string reason;

  KillEntityRequestEvent(Registery::Entity t, std::string r)
      : target(t)
      , reason(std::move(r))
  {
  }

  KillEntityRequestEvent(Registery& r, JsonObject const& e)
      : target(static_cast<Registery::Entity>(
            get_value_copy<double>(r, e, "target").value()))
      , reason(get_value_copy<std::string>(r, e, "reason").value_or(""))
  {
  }
};

struct ModifyComponentRequestEvent
{
  Registery::Entity target;
  std::string component_name;
  JsonObject modifications;

  ModifyComponentRequestEvent(Registery::Entity t,
                              std::string comp,
                              JsonObject mods)
      : target(t)
      , component_name(std::move(comp))
      , modifications(std::move(mods))
  {
  }

  ModifyComponentRequestEvent(Registery& r, JsonObject const& e)
      : target(static_cast<Registery::Entity>(
            get_value_copy<double>(r, e, "target").value()))
      , component_name(get_value_copy<std::string>(r, e, "component").value())
      , modifications(get_value_copy<JsonObject>(r, e, "modifications").value())
  {
  }
};

struct TimerTickEvent
{
  double delta_time;
  std::chrono::steady_clock::time_point now;
};
