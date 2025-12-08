#pragma once

#include <thread>
#include <vector>

#include <asio/io_context.hpp>
#include <asio/ip/udp.hpp>

#include "NetworkShared.hpp"
#include "plugin/Byte.hpp"
#include "ecs/Registery.hpp"
#include "plugin/APlugin.hpp"
#include "ClientConnection.hpp"
#include "plugin/EntityLoader.hpp"

class NetworkClient : public APlugin
{
  public:
    NetworkClient(Registery& r, EntityLoader& l);
    ~NetworkClient() override;

  private:
    void connection_thread(ClientConnection const& c);

    SharedQueue _component_queue;
    std::vector<std::thread> _threads;
    std::atomic<bool> _running = false;
};
