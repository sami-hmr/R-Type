#pragma once

#include <optional>
#include <string>

#include "ecs/Registry.hpp"
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
};

struct ModifyComponentRequestEvent
{
  Registry::Entity target;
  std::string component_name;
  JsonObject modifications;

  ModifyComponentRequestEvent(Registry::Entity t,
                              std::string comp,
                              JsonObject mods)
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
};

struct TimerTickEvent
{
  double delta_time;
  std::chrono::steady_clock::time_point now;
};
