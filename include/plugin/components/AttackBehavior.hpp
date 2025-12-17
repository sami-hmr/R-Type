#pragma once

#include <string>
#include <utility>

#include "ByteParser/ByteParser.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/events/EventMacros.hpp"

struct AttackBehavior
{
  AttackBehavior()
      : attack_type("continuous")
      , attack_delta(0.0)
      , attack_interval(2.0)
      , active(true)
  {
  }

  AttackBehavior(std::string type, double interval)
      : attack_type(std::move(type))
      , attack_delta(0.0)
      , attack_interval(interval)
      , active(true)
  {
  }

  AttackBehavior(std::string type,
                 double attack_delta,
                 double interval,
                 bool active)
      : attack_type(std::move(type))
      , attack_delta(attack_delta)
      , attack_interval(interval)
      , active(active)
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(AttackBehavior,
                           (
                               [](std::string attack_type, double interval)
                               {
                                 return AttackBehavior(
                                     std::string(attack_type.begin(),
                                                 attack_type.end()),
                                     interval);
                               }),
                           parseByteString(),
                           parseByte<double>())
  DEFAULT_SERIALIZE(string_to_byte(this->attack_type),
                    type_to_byte(this->attack_delta),
                    type_to_byte(this->attack_interval),
                    type_to_byte(this->active))

  CHANGE_ENTITY_DEFAULT

  std::string attack_type;
  double attack_delta;
  double attack_interval;
  bool active;

  HOOKABLE(AttackBehavior,
           HOOK(attack_type),
           HOOK(attack_delta),
           HOOK(attack_interval),
           HOOK(active))
};
