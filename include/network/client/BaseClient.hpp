#pragma once

#include <semaphore>
#include <set>
#include <thread>
#include <unordered_map>
#include <vector>

#include <asio/io_context.hpp>
#include <asio/ip/udp.hpp>

#include "ClientConnection.hpp"
#include "NetworkShared.hpp"
#include "TwoWayMap.hpp"
#include "ecs/Registry.hpp"
#include "network/client/Client.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"

class BaseClient : public APlugin
{
public:
  BaseClient(std::string const& name,
             Registry& r,
             EventManager& em,
             EntityLoader& l);
  ~BaseClient() override;

private:
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

private:
  std::unordered_set<Registry::Entity> _server_created;
};
