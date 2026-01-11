#include "ecs/Registry.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"

struct FetchAvailableServers {
    FetchAvailableServers() = default;

    EMPTY_BYTE_CONSTRUCTOR(FetchAvailableServers)
    DEFAULT_SERIALIZE(ByteArray {})

    CHANGE_ENTITY_DEFAULT

    FetchAvailableServers(Registry& /* */, JsonObject const& /* */) {}

    HOOKABLE(FetchAvailableServers)
};

struct ExposeServer
{
  std::string host;

  ExposeServer(std::string host)
  : host(std::move(host))
  {
  }

  ExposeServer(Registry& r, JsonObject const& e)
      : host(get_value_copy<std::string>(r, e, "host").value())
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(ExposeServer,
                           ([](std::string const& h)
                            { return ExposeServer(h); }),
                           parseByteString())

  DEFAULT_SERIALIZE(string_to_byte(host))

  CHANGE_ENTITY_DEFAULT
};
