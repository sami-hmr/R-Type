#pragma once

#include <thread>
#include <vector>

#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>

#include "ClientConnection.hpp"
#include "asio.hpp"
#include "ecs/Registery.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"

class NetworkClient : public APlugin
{
public:
  NetworkClient(Registery& r, EntityLoader& l)
      : APlugin(r, l, {}, {})
      , _resolver(this->_io_c)
  {
    this->_registery.get().on<ClientConnection>(
        [this](ClientConnection const& c)
        {
          this->_threads.emplace_back([this, c]() { this->reading_thread(c); });
        });
  }

  ~NetworkClient() override
  {
    for (auto& t : this->_threads) {
      t.join();
    }
  }

private:
  void reading_thread(ClientConnection const& c);

  asio::io_context _io_c;
  asio::ip::tcp::resolver _resolver;
  std::vector<std::thread> _threads;
};
