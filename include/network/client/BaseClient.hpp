#pragma once

#include <thread>
#include <vector>

#include <asio/io_context.hpp>
#include <asio/ip/udp.hpp>

#include "ClientConnection.hpp"
#include "NetworkShared.hpp"
#include "TwoWayMap.hpp"
#include "ecs/Registry.hpp"
#include "network/HttpClient.hpp"
#include "network/Httplib.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/events/HttpEvents.hpp"

class BaseClient : public APlugin
{
public:
  BaseClient(const BaseClient&) = delete;
  BaseClient(BaseClient&&) = delete;
  BaseClient& operator=(const BaseClient&) = delete;
  BaseClient& operator=(BaseClient&&) = delete;
  BaseClient(std::string const& name,
             std::string const& game_name,
             Registry& r,
             EventManager& em,
             EntityLoader& l);
  ~BaseClient() override;

  std::string const game_name;

private:
  void setup_http_requests();
  void connection_thread(ClientConnection const& c);
  SharedQueue<ComponentBuilder> _component_queue;
  SharedQueue<EventBuilder> _event_from_server;
  SharedQueue<EventBuilder> _event_to_server;

  // std::optional<Client> _client_class;
  std::thread _thread;
  std::atomic<bool> _running = false;
  bool _connected = false;

protected:
  TwoWayMap<Registry::Entity /*server */, Registry::Entity /*client */>
      _server_indexes;

  std::size_t _id_in_server = 0;

  HttpClient _http_client;

  int _user_id = -1;

  struct AvailableServer
  {
    std::size_t id;
    std::string address;
    std::size_t port;
  };

  std::vector<AvailableServer> _available_servers;

private:
  friend void handle_fetch_servers(void*, httplib::Result const&);
  void handle_server_fetch();
  friend void handle_login_response(void*, httplib::Result const&);
  void handle_register(Register const&);
  void handle_login(Login const&);

  std::unordered_set<Registry::Entity> _server_created;
};
