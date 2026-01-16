#pragma once

#include "Json/JsonParser.hpp"
#include "ecs/EventManager.hpp"
#include "ecs/Registry.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"

class Moving : public APlugin
{
public:
  Moving(Registry& r, EventManager& em, EntityLoader& l);

private:
  void init_pos(Registry::Entity const& entity, JsonObject& obj);
  void init_direction(Registry::Entity const& entity, JsonObject& obj);
  void init_speed(Registry::Entity const& entity, JsonObject& obj);
  void init_facing(Registry::Entity const& entity, JsonObject& obj);

  void init_id(Registry::Entity const& entity, JsonObject& obj);
  void moving_system(Registry&);
};
