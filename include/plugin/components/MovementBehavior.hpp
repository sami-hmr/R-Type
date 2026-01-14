#pragma once

#include <string>
#include <utility>

#include "ByteParser/ByteParser.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/events/EventMacros.hpp"

struct MovementBehavior
{
  MovementBehavior()
      : movement_type("straight")
  {
  }

  MovementBehavior(std::string type)
      : movement_type(std::move(type))
  {
  }

  MovementBehavior(std::string type, JsonObject p)
      : movement_type(std::move(type))
      , params(std::move(p))
  {
  }

  MovementBehavior(std::string type, double movement_delta, JsonObject p)
      : movement_type(std::move(type))
      , movement_delta(movement_delta)
      , params(std::move(p))
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(MovementBehavior,
                           (
                               [](std::string movement_type,
                                  double movement_delta,
                                  JsonObject params)
                               {
                                 return MovementBehavior(
                                     std::move(movement_type),
                                     movement_delta,
                                     std::move(params));
                               }),
                           parseByteString(),
                           parseByte<double>(),
                           parseByteJsonObject())
  DEFAULT_SERIALIZE(string_to_byte(this->movement_type),
                    type_to_byte(this->movement_delta),
                    json_object_to_byte(this->params))

  CHANGE_ENTITY_DEFAULT

  std::string movement_type;
  double movement_delta = 0.0;
  JsonObject params;

  HOOKABLE(MovementBehavior,
           HOOK(movement_type),
           HOOK(movement_delta),
           HOOK(params))
};
