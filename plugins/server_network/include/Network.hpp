#pragma once

// #include <functional>
// #include <map>
// #include <random>

#include <memory>
#include <thread>
#include <vector>

#include <asio/error_code.hpp>
#include <asio/io_context.hpp>
#include <asio/ip/udp.hpp>

#include "CustomException.hpp"
#include "ServerLaunch.hpp"
#include "ecs/Registery.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/Byte.hpp"
#include "plugin/EntityLoader.hpp"

class NetworkServer : public APlugin
{
  public:
    NetworkServer(Registery& r, EntityLoader& l);
    ~NetworkServer() override;

  private:
    void launch_server(ServerLaunching const& s);

    // std::reference_wrapper<Server> _server;
    std::queue<std::shared_ptr<ByteArray>> _components_to_create;
    std::vector<std::thread> _threads;
    bool _running = false;
};

CUSTOM_EXCEPTION(ClientNotFound)
