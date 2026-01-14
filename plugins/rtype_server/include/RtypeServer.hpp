#pragma once

// #include <functional>
// #include <map>
// #include <random>

#include <unordered_map>

#include <asio/error_code.hpp>
#include <asio/io_context.hpp>
#include <asio/ip/udp.hpp>

#include "ecs/Registry.hpp"
#include "network/server/BaseServer.hpp"
#include "plugin/EntityLoader.hpp"

class RtypeServer : public BaseServer
{
public:
  RtypeServer(Registry& r, EventManager& em, EntityLoader& l);
  ~RtypeServer() override = default;

private:
  std::map<int, std::size_t> _users_entities;

  friend void handle_get_player_save(void* raw_context,
                                     httplib::Result const& result);
  void ask_player_save(int user_id);
  void save_player(int user_id);
  std::unordered_map<std::size_t, bool> _player_ready;
  std::unordered_map<Registry::Entity, size_t> _player_entities;
};
