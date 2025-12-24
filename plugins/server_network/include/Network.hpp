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
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"

class NetworkServer : public APlugin
{
  public:
    NetworkServer(Registry& r, EventManager &em, EntityLoader& l);
    ~NetworkServer() override;

  private:
    void launch_server(ServerLaunching const& s);


    std::thread _thread;
    std::counting_semaphore<> _comp_semaphore;
    SharedQueue<ComponentBuilderId> _components_to_update;
    std::atomic<bool> _running = false;
    std::counting_semaphore<> _semaphore_event_to_server;
    SharedQueue<EventBuilder> _event_queue;
    std::counting_semaphore<> _event_semaphore;
    SharedQueue<EventBuilderId> _event_queue_to_client;
    std::unordered_map<std::size_t, bool> _player_ready;
    std::unordered_map<Registry::Entity, size_t> _player_entities;
};

CUSTOM_EXCEPTION(ClientNotFound)
