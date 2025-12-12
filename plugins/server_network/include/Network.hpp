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
#include "ecs/Registry.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"

class NetworkServer : public APlugin
{
  public:
    NetworkServer(Registry& r, EntityLoader& l);
    ~NetworkServer() override;

  private:
    void launch_server(ServerLaunching const& s);

    std::thread _thread;
    std::counting_semaphore<> _comp_semaphore;
    SharedQueue<ComponentBuilderId> _components_to_update;
    std::atomic<bool> _running = false;
    SharedQueue<EventBuilder> _event_queue;
    std::counting_semaphore<> _event_semaphore;
    SharedQueue<EventBuilderId> _event_queue_to_client;
};

CUSTOM_EXCEPTION(ClientNotFound)
