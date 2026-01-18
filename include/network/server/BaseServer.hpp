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
#include "network/HttpClient.hpp"
#include "network/Httplib.hpp"
#include "network/server/Server.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"

class BaseServer : public APlugin
{
public:
  BaseServer(std::string const& name,
             std::string game_name,
             Registry& r,
             EventManager& em,
             EntityLoader& l,
             std::optional<JsonObject> const& config);
  ~BaseServer() override;

  std::string const game_name;

private:
  void setup_http_requests();
  void launch_server(ServerLaunching const& infos);

  int _server_id = -1;
  int _port = -1;

  std::optional<Server> _server_class;
  std::thread _actual_server;
  SharedQueue<ComponentBuilderId> _components_to_update;
  std::atomic<bool> _running = false;
  SharedQueue<EventBuilder> _event_queue;
  SharedQueue<EventBuilderId> _event_queue_to_client;

protected:
  friend void handle_register_response(void*, httplib::Result const&);
  void register_server(std::string const& host);
  void unregister_server();

  HttpClient _http_client;

  int get_user_by_client(std::size_t client_id);
  int get_client_by_user(int user);
};
