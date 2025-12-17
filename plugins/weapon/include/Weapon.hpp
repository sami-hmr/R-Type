#pragma once

#include "ecs/Registry.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/events/WeaponEvent.hpp"
#include "plugin/events/IoEvents.hpp"

class Weapon : public APlugin
{
public:
  Weapon(Registry& r, EntityLoader& l);

  EntityLoader &entity_loader;

  private:
  void init_basic_weapon(Registry::Entity const &entity, JsonObject const &obj);
  void on_fire(Registry &r, const FireBullet &e);
  void basic_weapon_system(std::chrono::high_resolution_clock::time_point now);

};
