#pragma once

#include "ecs/EventManager.hpp"
#include "ecs/Registry.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/events/IoEvents.hpp"
#include "plugin/events/WeaponEvent.hpp"

class Weapon : public APlugin
{
public:
  Weapon(Registry& r, EventManager& em, EntityLoader& l);

  EntityLoader& entity_loader;

private:
  void init_basic_weapon(Ecs::Entity const& entity, JsonObject const& obj);
  void on_fire(Registry& r, const FireBullet& e);
  void basic_weapon_system(std::chrono::high_resolution_clock::time_point now);
  void init_charge_weapon(Ecs::Entity const& entity,
                          JsonObject const& obj);
  void init_delayed_weapon(Ecs::Entity const& entity,
                           JsonObject const& obj);
  void on_charge_start(Registry& r, const StartChargeWeapon& e);
  void on_charge_release(Registry& r, const ReleaseChargeWeapon& e);
  void charge_weapon_system(std::chrono::high_resolution_clock::time_point now);
  void delayed_weapon_system(
      std::chrono::high_resolution_clock::time_point now);
  void apply_scale_modifiers();
};
