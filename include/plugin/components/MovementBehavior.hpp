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
      , movement_delta(0.0)
  {
  }

  MovementBehavior(std::string type)
      : movement_type(std::move(type))
      , movement_delta(0.0)
  {
  }

  MovementBehavior(std::string type, double movement_delta)
      : movement_type(std::move(type))
      , movement_delta(movement_delta)
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(MovementBehavior,
                           (
                               [](std::vector<char> movement_type, double dt)
                               {
                                 return MovementBehavior(
                                     std::string(movement_type.begin(),
                                                 movement_type.end()),
                                     dt);
                               }),
                           parseByteArray(parseAnyChar()),
                           parseByte<double>())
  DEFAULT_SERIALIZE(string_to_byte(this->movement_type),
                    type_to_byte(this->movement_delta))

  CHANGE_ENTITY_DEFAULT

  std::string movement_type;
  double movement_delta;

  HOOKABLE(MovementBehavior, HOOK(movement_type), HOOK(movement_delta))
};
