#pragma once

#include <chrono>
#include <string>

/**
 * Base weapon struct containing common properties shared by all weapon types
 */
struct BaseWeapon
{
  std::string bullet_type;
  int magazine_size;
  int magazine_nb;
  int remaining_ammo;
  int remaining_magazine;
  double reload_time;
  double cooldown;
  std::string attack_animation;
  bool reloading = false;
  std::chrono::high_resolution_clock::time_point last_shot_time;
  std::chrono::high_resolution_clock::time_point last_reload_time;

  /**
   * Common weapon update logic for cooldown and ammo management
   * @param now Current time point
   * @return true if weapon can fire, false otherwise
   */
  bool update_weapon(std::chrono::high_resolution_clock::time_point now)
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

protected:
  BaseWeapon(std::string bullet_type,
             int magazine_size,
             int magazine_nb,
             double reload_time,
             double cooldown,
             std::string attack_animation)
      : bullet_type(std::move(bullet_type))
      , magazine_size(magazine_size)
      , magazine_nb(magazine_nb)
      , remaining_ammo(magazine_size)
      , remaining_magazine(magazine_nb)
      , reload_time(reload_time)
      , cooldown(cooldown)
      , attack_animation(std::move(attack_animation))
  {
  }

  BaseWeapon() = default;
};
