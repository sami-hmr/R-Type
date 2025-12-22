#pragma once

// #include <functional>
// #include <map>
// #include <random>

#include <memory>
#include <semaphore>
#include <thread>
#include <unordered_map>
#include <vector>

#include <asio/error_code.hpp>
#include <asio/io_context.hpp>
#include <asio/ip/udp.hpp>

#include "CustomException.hpp"
#include "NetworkShared.hpp"
#include "ServerLaunch.hpp"
#include "ecs/Registry.hpp"
#include "network/server/BaseServer.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"

class RtypeServer : public BaseServer
{
  public:
    RtypeServer(Registry& r, EventManager &em, EntityLoader& l);
    ~RtypeServer() override = default;

  private:
    std::unordered_map<std::size_t, bool> _player_ready;
    std::unordered_map<Registry::Entity, size_t> _player_entities;
};

CUSTOM_EXCEPTION(ClientNotFound)
