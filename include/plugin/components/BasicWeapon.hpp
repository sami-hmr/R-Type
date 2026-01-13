#pragma once

#include <string>
#include <utility>

#include "BaseTypes.hpp"
#include "ByteParser/ByteParser.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/events/EventMacros.hpp"
#include "plugin/events/IoEvents.hpp"

struct BasicWeapon
{
  BasicWeapon() = default;

  BasicWeapon(std::string bullet_type,
              int magazine_size,
              int magazine_nb,
              double reload_time,
              double cooldown,
              std::string attack_animation = "")
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

  DEFAULT_BYTE_CONSTRUCTOR(BasicWeapon,
                           (
                               [](std::string bullet_type,
                                  int mag_size,
                                  int mag_nb,
                                  double reload_time,
                                  double cooldown,
                                  std::string attack_animation)
                               {
                                 return BasicWeapon(bullet_type,
                                                    mag_size,
                                                    mag_nb,
                                                    reload_time,
                                                    cooldown,
                                                    attack_animation);
                               }),
                           parseByteString(),
                           parseByte<int>(),
                           parseByte<int>(),
                           parseByte<double>(),
                           parseByte<double>(),
                           parseByteString())
  DEFAULT_SERIALIZE(string_to_byte(this->bullet_type),
                    type_to_byte(this->magazine_size),
                    type_to_byte(this->magazine_nb),
                    type_to_byte(this->reload_time),
                    type_to_byte(this->cooldown),
                    string_to_byte(this->attack_animation))
  CHANGE_ENTITY_DEFAULT

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

  HOOKABLE(BasicWeapon,
           HOOK(bullet_type),
           HOOK(magazine_size),
           HOOK(magazine_nb),
           HOOK(remaining_ammo),
           HOOK(remaining_magazine),
           HOOK(reload_time),
           HOOK(reloading),
           HOOK(last_reload_time),
           HOOK(cooldown),
           HOOK(attack_animation))

  bool update_basic_weapon(std::chrono::high_resolution_clock::time_point now);
};

struct ChargeWeapon
{
  ChargeWeapon() = default;

  ChargeWeapon(std::string bullet_type,
               int magazine_size,
               int magazine_nb,
               double reload_time,
               double cooldown,
               double charge_time,
               double max_scale,
               double min_charge_threshold,
               bool scale_damage,
               std::string attack_animation = "",
               std::string charge_indicator = "")
      : bullet_type(std::move(bullet_type))
      , magazine_size(magazine_size)
      , magazine_nb(magazine_nb)
      , remaining_ammo(magazine_size)
      , remaining_magazine(magazine_nb)
      , reload_time(reload_time)
      , cooldown(cooldown)
      , charge_time(charge_time)
      , max_scale(max_scale)
      , min_charge_threshold(min_charge_threshold)
      , scale_damage(scale_damage)
      , attack_animation(std::move(attack_animation))
      , charge_indicator(std::move(charge_indicator))
      , charge_indicator_entity(static_cast<Registry::Entity>(-1))
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(ChargeWeapon,
                           (
                               [](std::string bullet_type,
                                  int mag_size,
                                  int mag_nb,
                                  double reload_time,
                                  double cooldown,
                                  double charge_time,
                                  double max_scale,
                                  double min_charge_threshold,
                                  bool scale_damage,
                                  std::string attack_animation,
                                  std::string charge_indicator)
                               {
                                 return ChargeWeapon(bullet_type,
                                                     mag_size,
                                                     mag_nb,
                                                     reload_time,
                                                     cooldown,
                                                     charge_time,
                                                     max_scale,
                                                     min_charge_threshold,
                                                     scale_damage,
                                                     attack_animation,
                                                     charge_indicator);
                               }),
                           parseByteString(),
                           parseByte<int>(),
                           parseByte<int>(),
                           parseByte<double>(),
                           parseByte<double>(),
                           parseByte<double>(),
                           parseByte<double>(),
                           parseByte<double>(),
                           parseByte<bool>(),
                           parseByteString(),
                           parseByteString())
  DEFAULT_SERIALIZE(string_to_byte(this->bullet_type),
                    type_to_byte(this->magazine_size),
                    type_to_byte(this->magazine_nb),
                    type_to_byte(this->reload_time),
                    type_to_byte(this->cooldown),
                    type_to_byte(this->charge_time),
                    type_to_byte(this->max_scale),
                    type_to_byte(this->min_charge_threshold),
                    type_to_byte(this->scale_damage),
                    string_to_byte(this->attack_animation),
                    string_to_byte(this->charge_indicator))
  CHANGE_ENTITY(
      result.charge_indicator_entity = map.at(charge_indicator_entity))

  std::string bullet_type;
  int magazine_size;
  int magazine_nb;
  int remaining_ammo;
  int remaining_magazine;
  double reload_time;
  double cooldown;
  double charge_time;
  double max_scale;
  double min_charge_threshold;
  bool scale_damage;
  std::string attack_animation;
  std::string charge_indicator;
  bool reloading = false;
  std::chrono::high_resolution_clock::time_point last_shot_time;
  std::chrono::high_resolution_clock::time_point last_reload_time;

  // Charging state
  bool is_charging = false;
  double current_charge_level = 0.0;
  std::chrono::high_resolution_clock::time_point charge_start_time;
  std::size_t charge_indicator_entity = static_cast<Registry::Entity>(-1);
  Vector2D charge_indicator_base_scale = Vector2D(1.0, 1.0);

  HOOKABLE(ChargeWeapon,
           HOOK(bullet_type),
           HOOK(magazine_size),
           HOOK(magazine_nb),
           HOOK(remaining_ammo),
           HOOK(remaining_magazine),
           HOOK(reload_time),
           HOOK(reloading),
           HOOK(last_reload_time),
           HOOK(cooldown),
           HOOK(charge_time),
           HOOK(max_scale),
           HOOK(min_charge_threshold),
           HOOK(scale_damage),
           HOOK(attack_animation),
           HOOK(charge_indicator))

  bool update_basic_weapon(std::chrono::high_resolution_clock::time_point now);
};

struct DelayedWeapon
{
  DelayedWeapon() = default;

  DelayedWeapon(std::string bullet_type,
                int magazine_size,
                int magazine_nb,
                double reload_time,
                double cooldown,
                double delay_time,
                std::string attack_animation = "")
      : bullet_type(std::move(bullet_type))
      , magazine_size(magazine_size)
      , magazine_nb(magazine_nb)
      , remaining_ammo(magazine_size)
      , remaining_magazine(magazine_nb)
      , reload_time(reload_time)
      , cooldown(cooldown)
      , delay_time(delay_time)
      , attack_animation(std::move(attack_animation))
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

  std::string bullet_type;
  int magazine_size;
  int magazine_nb;
  int remaining_ammo;
  int remaining_magazine;
  double reload_time;
  double cooldown;
  double delay_time;
  std::string attack_animation;
  bool reloading = false;
  std::chrono::high_resolution_clock::time_point last_shot_time;
  std::chrono::high_resolution_clock::time_point last_reload_time;
  std::chrono::high_resolution_clock::time_point pending_shot_time;
  bool has_pending_shot = false;

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

  bool update_basic_weapon(std::chrono::high_resolution_clock::time_point now);
};
