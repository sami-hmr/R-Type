#pragma once

#include <semaphore>
#include <thread>
#include <unordered_map>
#include <vector>

#include <asio/io_context.hpp>
#include <asio/ip/udp.hpp>

#include "NetworkShared.hpp"
#include "TwoWayMap.hpp"
#include "ecs/Registry.hpp"
#include "plugin/APlugin.hpp"
#include "ClientConnection.hpp"
#include "plugin/EntityLoader.hpp"

class NetworkClient : public APlugin
{
  public:
    NetworkClient(Registry& r, EntityLoader& l);
    ~NetworkClient() override;

  private:
    void connection_thread(ClientConnection const& c);

    SharedQueue<ComponentBuilder> _component_queue;
    SharedQueue<EventBuilder> _exec_event_queue;

    TwoWayMap<Registry::Entity, Registry::Entity> _server_indexes;

    std::counting_semaphore<> _sem;
    SharedQueue<EventBuilder> _event_queue;
    std::thread _thread;

    std::atomic<bool> _running = false;
};
