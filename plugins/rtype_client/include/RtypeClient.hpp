#pragma once

#include <vector>
#include <asio/io_context.hpp>
#include <asio/ip/udp.hpp>

#include "ecs/Registry.hpp"
#include "network/client/BaseClient.hpp"
#include "plugin/EntityLoader.hpp"

class RtypeClient : public BaseClient
{
public:
  RtypeClient(Registry& r, EventManager& em, EntityLoader& l);
  ~RtypeClient() override = default;

private:
  void handle_http();

  void alert(std::string const &message);
  void handle_server_fetched();
  std::string _current_server_fetch_scene;
  std::vector<std::size_t> _server_fetch_entities;
};
