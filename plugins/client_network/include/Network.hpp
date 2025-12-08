#pragma once

#include <thread>
#include <vector>

#include <asio/io_context.hpp>
#include <asio/ip/udp.hpp>

#include "NetworkShared.hpp"
#include "plugin/Byte.hpp"
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

    SharedQueue<ComponentBuilder> _components_to_update;
    SharedQueue<EventBuilder> _events_to_transmit;
    std::vector<std::thread> _threads;
    std::atomic<bool> _running = false;
};
