#pragma once

#include "ecs/Registry.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/events/IoEvents.hpp"

class Weapon : public APlugin
{
public:
  Weapon(Registry& r, EntityLoader& l);

  EntityLoader &entity_loader;
  
  private:
  void init_basic_weapon(Registry::Entity const &entity, JsonObject const &obj);
  void on_fire(Registry &r, const KeyPressedEvent &e);
  
};
