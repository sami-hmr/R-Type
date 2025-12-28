#pragma once

// #include <functional>
// #include <map>
// #include <random>

#include <memory>
#include <optional>
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
#include "network/server/Server.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"

class BaseServer : public APlugin
{
  public:
    BaseServer(std::string const &name, Registry& r, EventManager &em, EntityLoader& l);
    ~BaseServer() override;

  private:
    void launch_server(ServerLaunching const& s);

    std::optional<Server> _server_class;
    std::thread _actual_server;
    SharedQueue<ComponentBuilderId> _components_to_update;
    std::atomic<bool> _running = false;
    SharedQueue<EventBuilder> _event_queue;
    SharedQueue<EventBuilderId> _event_queue_to_client;
};
