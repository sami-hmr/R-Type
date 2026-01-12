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
      , active(true)
  {
  }

  AttackBehavior(std::string type, JsonObject params)
      : attack_type(std::move(type))
      , attack_delta(0.0)
      , active(true)
      , params(std::move(params))
  {
  }

  AttackBehavior(std::string type,
                 double attack_delta,
                 bool active,
                 JsonObject params)
      : attack_type(std::move(type))
      , attack_delta(attack_delta)
      , active(active)
      , params(std::move(params))
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(AttackBehavior,
                           (
                               [](std::string attack_type,
                                  double attack_delta,
                                  bool active,
                                  JsonObject params)
                               {
                                 return AttackBehavior(std::move(attack_type),
                                                       attack_delta,
                                                       active,
                                                       std::move(params));
                               }),
                           parseByteString(),
                           parseByte<double>(),
                           parseByte<bool>(),
                           parseByteJsonObject())
  DEFAULT_SERIALIZE(string_to_byte(this->attack_type),
                    type_to_byte(this->attack_delta),
                    type_to_byte(this->active),
                    json_object_to_byte(this->params))

  CHANGE_ENTITY_DEFAULT

  std::string attack_type;
  double attack_delta;
  bool active;
  JsonObject params;

  HOOKABLE(AttackBehavior, HOOK(attack_type), HOOK(attack_delta), HOOK(active), HOOK(params))
};
