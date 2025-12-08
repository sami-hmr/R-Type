#pragma once

// #include <functional>
// #include <map>
// #include <random>

#include <memory>
#include <semaphore>
#include <thread>
#include <vector>

#include <asio/error_code.hpp>
#include <asio/io_context.hpp>
#include <asio/ip/udp.hpp>

#include "CustomException.hpp"
#include "NetworkShared.hpp"
#include "ServerLaunch.hpp"
#include "ecs/Registery.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"

class NetworkServer : public APlugin
{
  public:
    NetworkServer(Registery& r, EntityLoader& l);
    ~NetworkServer() override;

  private:
    void launch_server(ServerLaunching const& s);

    // std::reference_wrapper<Server> _server;
    std::vector<std::thread> _threads;
    SharedQueue<ComponentBuilder> _component_queue;
    std::atomic<bool> _running = false;
    SharedQueue<EventBuilder> _event_queue;
    std::counting_semaphore<> _event_semaphore;
};

CUSTOM_EXCEPTION(ClientNotFound)
