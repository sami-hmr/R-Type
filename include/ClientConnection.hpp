#pragma once

#include <string>

#include "ByteParser/ByteParser.hpp"
#include "TwoWayMap.hpp"
#include "ecs/Registry.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/events/EventMacros.hpp"

struct ClientConnection
{
  std::string host;
  std::size_t port;

  ClientConnection() = default;

  ClientConnection(std::string h, std::size_t p)
      : host(std::move(h))
      , port(p)
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(
      ClientConnection,
      [](std::string const& h, std::size_t p)
      { return ClientConnection(h, p); },
      parseByteString(),
      parseByte<std::size_t>())
  DEFAULT_SERIALIZE(string_to_byte(host), type_to_byte(port))

  CHANGE_ENTITY_DEFAULT

  ClientConnection(Registry& r, JsonObject const& e)
      : host(get_value_copy<std::string>(r, e, "host").value())
      , port(get_value_copy<std::size_t>(r, e, "port").value())
  {
  }
};
