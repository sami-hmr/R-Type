#pragma once

#include "Json/JsonParser.hpp"
#include "ecs/Registry.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"

class Actions : public APlugin
{
public:
  Actions(Registry& r, EventManager& em, EntityLoader& l);

private:
  void init_action_trigger(Registry::Entity const& entity, JsonObject& obj);
};
