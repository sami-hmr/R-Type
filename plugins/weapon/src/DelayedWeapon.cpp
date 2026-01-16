#include "Weapon.hpp"
#include "ecs/InitComponent.hpp"
#include "ecs/Registry.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/components/BasicWeapon.hpp"

void Weapon::init_delayed_weapon(Registry::Entity const& entity,
                                 JsonObject const& obj)
{
  auto const& bullet_type = get_value<DelayedWeapon, std::string>(
      this->_registry.get(), obj, entity, "bullet_type");
  if (!bullet_type) {
    std::cerr << "Error loading DelayedWeapon component: unexpected value type "
                     "(bullet_type: string)\n";
    return;
  }

  auto const& magazine_size = get_value<DelayedWeapon, int>(
      this->_registry.get(), obj, entity, "magazine_size");
  if (!magazine_size) {
    std::cerr << "Error loading DelayedWeapon component: unexpected value type "
                     "(magazine_size: int)\n";
    return;
  }

  auto const& magazine_nb = get_value<DelayedWeapon, int>(
      this->_registry.get(), obj, entity, "magazine_nb");
  if (!magazine_nb) {
    std::cerr << "Error loading DelayedWeapon component: unexpected value type "
                     "(magazine_nb: int)\n";
    return;
  }
  auto const& reload_time = get_value<DelayedWeapon, double>(
      this->_registry.get(), obj, entity, "reload_time");
  if (!reload_time) {
    std::cerr << "Error loading DelayedWeapon component: unexpected value type "
                     "(reload_time: double)\n";
    return;
  }
  auto const& cooldown = get_value<DelayedWeapon, double>(
      this->_registry.get(), obj, entity, "cooldown");
  if (!cooldown) {
    std::cerr << "Error loading DelayedWeapon component: unexpected value type "
                     "(cooldown: double)\n";
    return;
  }
  auto const& delay_time = get_value<DelayedWeapon, double>(
      this->_registry.get(), obj, entity, "delay_time");
  if (!delay_time) {
    std::cerr << "Error loading DelayedWeapon component: unexpected value type "
                     "(delay_time: double)\n";
    return;
  }

  auto const& attack_animation = get_value<DelayedWeapon, std::string>(
      this->_registry.get(), obj, entity, "attack_animation");
  std::string attack_anim_name =
      attack_animation ? attack_animation.value() : "";

  init_component<DelayedWeapon>(this->_registry.get(),
                                this->_event_manager.get(),
                                entity,
                                bullet_type.value(),
                                magazine_size.value(),
                                magazine_nb.value(),
                                reload_time.value(),
                                cooldown.value(),
                                delay_time.value(),
                                attack_anim_name);
}

bool DelayedWeapon::update_basic_weapon(
    std::chrono::high_resolution_clock::time_point now)
{
  if (this->reloading) {
    return false;
  }
  if (this->remaining_ammo <= 0) {
    return false;
  }
  double elapsed_time =
      std::chrono::duration<double>(now - this->last_shot_time).count();
  if (elapsed_time < this->cooldown) {
    return false;
  }
  this->last_shot_time = now;
  this->remaining_ammo -= 1;
  if (this->remaining_ammo <= 0 && this->remaining_magazine > 0) {
    this->reloading = true;
    this->last_reload_time = std::chrono::high_resolution_clock::now();
  }
  return true;
}
