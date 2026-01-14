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
  {
  }

  AttackBehavior(std::string type, JsonObject params)
      : attack_type(std::move(type))
      , params(std::move(params))
  {
  }

  AttackBehavior(std::string type,
                 double attack_delta,
                 double last_update,
                 bool active,
                 JsonObject params)
      : attack_type(std::move(type))
      , attack_delta(attack_delta)
      , last_update(last_update)
      , active(active)
      , params(std::move(params))
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(AttackBehavior,
                           (
                               [](std::string attack_type,
                                  double attack_delta,
                                  double last_update,
                                  bool active,
                                  JsonObject params)
                               {
                                 return AttackBehavior(std::move(attack_type),
                                                       attack_delta,
                                                       last_update,
                                                       active,
                                                       std::move(params));
                               }),
                           parseByteString(),
                           parseByte<double>(),
                           parseByte<double>(),
                           parseByte<bool>(),
                           parseByteJsonObject())
  DEFAULT_SERIALIZE(string_to_byte(this->attack_type),
                    type_to_byte(this->attack_delta),
                    type_to_byte(this->last_update),
                    type_to_byte(this->active),
                    json_object_to_byte(this->params))

  CHANGE_ENTITY_DEFAULT

  std::string attack_type;
  double attack_delta = 0.0;
  double last_update = 0.0;
  bool active = true;
  JsonObject params;

  HOOKABLE(AttackBehavior,
           HOOK(attack_type),
           HOOK(attack_delta),
           HOOK(last_update),
           HOOK(active),
           HOOK(params))
};
