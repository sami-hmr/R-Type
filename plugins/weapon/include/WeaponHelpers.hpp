#pragma once

#include "NetworkShared.hpp"
#include "Weapon.hpp"
#include "ecs/zipper/ZipperIndex.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/components/AnimatedSprite.hpp"
#include "plugin/events/AnimationEvents.hpp"

template<typename WeaponType>
typename Weapon::BaseWeaponFields<WeaponType> Weapon::parse_base_weapon_fields(
    Ecs::Entity const& entity, JsonObject const& obj)
{
  BaseWeaponFields<WeaponType> fields;
  fields.valid = true;

  // Parse bullet_type (required)
  fields.bullet_type = get_value<WeaponType, std::string>(
      this->_registry.get(), obj, entity, "bullet_type");
  if (!fields.bullet_type) {
    std::cerr << "Error loading weapon component: unexpected value type "
                 "(bullet_type: string)\n";
    fields.valid = false;
    return fields;
  }

  // Parse magazine_size (required)
  fields.magazine_size = get_value<WeaponType, int>(
      this->_registry.get(), obj, entity, "magazine_size");
  if (!fields.magazine_size) {
    std::cerr << "Error loading weapon component: unexpected value type "
                 "(magazine_size: int)\n";
    fields.valid = false;
    return fields;
  }

  // Parse magazine_nb (required)
  fields.magazine_nb = get_value<WeaponType, int>(
      this->_registry.get(), obj, entity, "magazine_nb");
  if (!fields.magazine_nb) {
    std::cerr << "Error loading weapon component: unexpected value type "
                 "(magazine_nb: int)\n";
    fields.valid = false;
    return fields;
  }

  // Parse reload_time (required)
  fields.reload_time = get_value<WeaponType, double>(
      this->_registry.get(), obj, entity, "reload_time");
  if (!fields.reload_time) {
    std::cerr << "Error loading weapon component: unexpected value type "
                 "(reload_time: double)\n";
    fields.valid = false;
    return fields;
  }

  // Parse cooldown (required)
  fields.cooldown = get_value<WeaponType, double>(
      this->_registry.get(), obj, entity, "cooldown");
  if (!fields.cooldown) {
    std::cerr << "Error loading weapon component: unexpected value type "
                 "(cooldown: double)\n";
    fields.valid = false;
    return fields;
  }

  // Parse optional fields
  fields.attack_animation = get_value<WeaponType, std::string>(
      this->_registry.get(), obj, entity, "attack_animation");

  fields.offset_x = get_value<WeaponType, double>(
      this->_registry.get(), obj, entity, "offset_x");

  fields.offset_y = get_value<WeaponType, double>(
      this->_registry.get(), obj, entity, "offset_y");

  return fields;
}

inline void Weapon::try_play_attack_animation(Ecs::Entity entity,
                                              const std::string& animation_name)
{
  if (animation_name.empty()
      || !this->_registry.get().has_component<AnimatedSprite>(entity))
  {
    return;
  }

  auto& sprite =
      *this->_registry.get().get_components<AnimatedSprite>()[entity];

  if (sprite.animations.contains(animation_name)) {
    auto& attack_anim = sprite.animations[animation_name];
    this->_event_manager.get().emit<PlayAnimationEvent>(animation_name,
                                                        entity,
                                                        attack_anim.framerate,
                                                        attack_anim.loop,
                                                        attack_anim.rollback);
  }
}

template<typename WeaponType>
void Weapon::emit_weapon_component_update(Ecs::Entity entity,
                                          const WeaponType& weapon)
{
  this->_event_manager.get().emit<ComponentBuilder>(
      entity,
      this->_registry.get().get_component_key<WeaponType>(),
      weapon.to_bytes());
}

template<typename WeaponType>
void Weapon::handle_reload_system(
    std::chrono::high_resolution_clock::time_point now)
{
  for (auto&& [entity, weapon] : ZipperIndex<WeaponType>(this->_registry.get()))
  {
    if (weapon.reloading && weapon.remaining_magazine > 0) {
      double elapsed_time =
          std::chrono::duration<double>(now - weapon.last_reload_time).count();
      if (elapsed_time >= weapon.reload_time) {
        weapon.reloading = false;
        weapon.remaining_ammo = weapon.magazine_size;
        weapon.remaining_magazine -= 1;

        // Emit ComponentBuilder for reload state change
        emit_weapon_component_update(entity, weapon);
      }
    }
  }
}
