#pragma once

#include <cstddef>
#include <unordered_map>

#include "Json/JsonParser.hpp"
#include "ecs/Registry.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/components/ActionTrigger.hpp"
#include "plugin/events/ActionEvents.hpp"
#include "plugin/events/Events.hpp"

class Actions : public APlugin
{
public:
  Actions(Registry& r, EntityLoader& l);

private:
  void init_action_trigger(Registry::Entity const& entity, JsonObject& obj);
};
