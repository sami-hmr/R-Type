#pragma once

#include "ecs/EventManager.hpp"
#include "ecs/Registry.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/events/CollisionEvent.hpp"


class Raycasting : public APlugin
{
public:
  Raycasting(Registry &r, EventManager &em, EntityLoader &l, std::optional<JsonObject> const &config);
  
  bool on_update_direction(Registry &r, const UpdateDirection &event);


  void init_basic_map(Registry::Entity& e, const JsonObject& obj);
  void init_cam(Registry::Entity& e, const JsonObject& obj);

  bool changed_direction = false;
};
