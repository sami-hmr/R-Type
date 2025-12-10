#pragma once

#include <string>

#include "ByteParser/ByteParser.hpp"
#include "TwoWayMap.hpp"
#include "ecs/Registry.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/events/EventMacros.hpp"

struct ServerLaunching
{
  std::size_t port;

  ServerLaunching(std::size_t p)
      : port(p)
  {
  }

  ServerLaunching(Registry& r, JsonObject const& e)
      : port(get_value_copy<std::size_t>(r, e, "port").value())
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(ServerLaunching,
                           ([](double p) { return ServerLaunching(p); }),
                           parseByte<std::size_t>())

  DEFAULT_SERIALIZE(type_to_byte(port))

  CHANGE_ENTITY_DEFAULT
};

struct SendMessage
{
  std::string message;

  SendMessage(std::string message)
      : message(std::move(message))
  {
  }

  SendMessage(Registry& r, JsonObject const& e)
      : message(get_value_copy<std::string>(r, e, "message").value())
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(SendMessage,
                           ([](std::string const &m) { return SendMessage(m); }),
                           parseByteString())

  DEFAULT_SERIALIZE(string_to_byte(message))

  CHANGE_ENTITY_DEFAULT
};
