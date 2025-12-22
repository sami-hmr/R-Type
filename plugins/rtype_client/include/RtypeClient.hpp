#pragma once

#include <asio/io_context.hpp>
#include <asio/ip/udp.hpp>
#include "ecs/Registry.hpp"
#include "network/client/BaseClient.hpp"
#include "plugin/EntityLoader.hpp"

class RtypeClient : public BaseClient
{
  public:
    RtypeClient(Registry& r, EventManager &em, EntityLoader& l);
    ~RtypeClient() override = default;

  private:
    std::size_t _id_in_server;
};
