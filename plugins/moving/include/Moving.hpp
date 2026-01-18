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
  void init_pos(Ecs::Entity const& entity, JsonObject& obj);
  void init_off(Ecs::Entity const& entity, JsonObject& obj);
  void init_direction(Ecs::Entity const& entity, JsonObject& obj);
  void init_speed(Ecs::Entity const& entity, JsonObject& obj);
  void init_facing(Ecs::Entity const& entity, JsonObject& obj);

  void init_id(Ecs::Entity const& entity, JsonObject& obj);
  void moving_system(Registry&);

  void add_offset(Registry& r);
  void remove_offset(Registry& r);
};
