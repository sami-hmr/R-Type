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
#include "NetworkShared.hpp"
#include "ServerLaunch.hpp"
#include "ecs/Registry.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/Byte.hpp"
#include "plugin/EntityLoader.hpp"

class NetworkServer : public APlugin
{
  public:
    NetworkServer(Registry& r, EntityLoader& l);
    ~NetworkServer() override;

  private:
    void launch_server(ServerLaunching const& s);

    std::vector<std::thread> _threads;
    SharedQueue<ComponentBuilder> _components_to_update;
    SharedQueue<EventBuilder> _events_to_transmit;
    std::atomic<bool> _running = false;
};

CUSTOM_EXCEPTION(ClientNotFound)
