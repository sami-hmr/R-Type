#pragma once

#include <chrono>
#include <string>

#include "BaseTypes.hpp"
#include "ByteParser/ByteParser.hpp"
#include "ecs/Registry.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/components/BaseWeapon.hpp"
#include "plugin/events/EventMacros.hpp"

struct DelayedWeapon : public BaseWeapon
{
  double delay_time;
  std::chrono::high_resolution_clock::time_point pending_shot_time;
  bool has_pending_shot = false;

  DelayedWeapon() = default;

  DelayedWeapon(std::string bullet_type,
                int magazine_size,
                int magazine_nb,
                double reload_time,
                double cooldown,
                double delay_time,
                std::string attack_animation = "")
      : BaseWeapon(std::move(bullet_type),
                   magazine_size,
                   magazine_nb,
                   reload_time,
                   cooldown,
                   std::move(attack_animation))
      , delay_time(delay_time)
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(DelayedWeapon,
                           (
                               [](std::string bullet_type,
                                  int mag_size,
                                  int mag_nb,
                                  double reload_time,
                                  double cooldown,
                                  double delay_time,
                                  std::string attack_animation)
                               {
                                 return DelayedWeapon(bullet_type,
                                                      mag_size,
                                                      mag_nb,
                                                      reload_time,
                                                      cooldown,
                                                      delay_time,
                                                      attack_animation);
                               }),
                           parseByteString(),
                           parseByte<int>(),
                           parseByte<int>(),
                           parseByte<double>(),
                           parseByte<double>(),
                           parseByte<double>(),
                           parseByteString())
  DEFAULT_SERIALIZE(string_to_byte(this->bullet_type),
                    type_to_byte(this->magazine_size),
                    type_to_byte(this->magazine_nb),
                    type_to_byte(this->reload_time),
                    type_to_byte(this->cooldown),
                    type_to_byte(this->delay_time),
                    string_to_byte(this->attack_animation))
  CHANGE_ENTITY_DEFAULT

  HOOKABLE(DelayedWeapon,
           HOOK(bullet_type),
           HOOK(magazine_size),
           HOOK(magazine_nb),
           HOOK(remaining_ammo),
           HOOK(remaining_magazine),
           HOOK(reload_time),
           HOOK(reloading),
           HOOK(last_reload_time),
           HOOK(cooldown),
           HOOK(delay_time),
           HOOK(attack_animation))

  /**
   * Update weapon state and check if it can fire
   * @param now Current time point
   * @return true if weapon can fire, false otherwise
   */
  bool update_basic_weapon(std::chrono::high_resolution_clock::time_point now)
  {
    return update_weapon(now);
  }
};
