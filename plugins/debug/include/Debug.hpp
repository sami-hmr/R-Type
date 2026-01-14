#pragma once

#include <optional>

#include "Json/JsonParser.hpp"
#include "ecs/EventManager.hpp"
#include "ecs/Registry.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"

class Debug : public APlugin
{
public:
  Debug(Registry& r,
        EventManager& em,
        EntityLoader& l,
        std::optional<JsonObject> const& config);

  // Basic debug system - prints entity component info
  void print_entity_info(Registry::Entity entity);

  // Print all active entities with all their component data
  void print_all_entities(Registry& r);

private:
  bool _enabled;
};
