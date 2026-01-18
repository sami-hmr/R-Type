#pragma once

#include "ecs/EventManager.hpp"
#include "ecs/Registry.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/events/WeaponEvent.hpp"

class Weapon : public APlugin
{
public:
  Weapon(Registry& r, EventManager& em, EntityLoader& l);

  EntityLoader& entity_loader;

private:
  void init_basic_weapon(Ecs::Entity const& entity, JsonObject const& obj);
  void on_fire(Registry& r, const FireBullet& e);

  void fire_basic(Registry& r, const FireBullet& e);
  void fire_delayed(Registry& r, const FireBullet& e);

  void basic_weapon_system(std::chrono::high_resolution_clock::time_point now);
  void init_charge_weapon(Ecs::Entity const& entity, JsonObject const& obj);
  void init_delayed_weapon(Ecs::Entity const& entity, JsonObject const& obj);
  void on_charge_start(Registry& r, const StartChargeWeapon& e);
  void on_charge_release(Registry& r, const ReleaseChargeWeapon& e);
  void charge_weapon_system(std::chrono::high_resolution_clock::time_point now);
  void delayed_weapon_system(
      std::chrono::high_resolution_clock::time_point now);
  void apply_scale_modifiers();

  // Helper functions to reduce code duplication
  template<typename WeaponType>
  struct BaseWeaponFields
  {
    std::optional<std::string> bullet_type;
    std::optional<int> magazine_size;
    std::optional<int> magazine_nb;
    std::optional<double> reload_time;
    std::optional<double> cooldown;
    std::optional<std::string> attack_animation;
    std::optional<double> offset_x;
    std::optional<double> offset_y;
    bool valid;
  };

  template<typename WeaponType>
  BaseWeaponFields<WeaponType> parse_base_weapon_fields(
      Ecs::Entity const& entity, JsonObject const& obj);

  void try_play_attack_animation(Ecs::Entity entity,
                                 const std::string& animation_name);

  template<typename WeaponType>
  void handle_reload_system(std::chrono::high_resolution_clock::time_point now);

  template<typename WeaponType>
  void emit_weapon_component_update(Ecs::Entity entity,
                                    const WeaponType& weapon);
};
