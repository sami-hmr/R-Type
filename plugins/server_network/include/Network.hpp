#pragma once

#include <thread>
#include <vector>
#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/registered_buffer.hpp>
#include <asio/write.hpp>

#include "Events.hpp"
#include "ServerLaunch.hpp"
#include "ecs/Registery.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"
#include "ServerLaunch.hpp"

class NetworkServer : public APlugin
{
public:
  NetworkServer(Registery& r, EntityLoader& l)
      : APlugin(r, l, {}, {})
  {
      this->_registery.get().on<ServerLaunching>([this](ServerLaunching const &s){
          this->_threads.emplace_back([this, s](){this->launch_server(s);});
    });
      this->_registery.get().on<SendMessage>([this](SendMessage const &m) {
          for (auto &it : this->_clients) {
              asio::write(it, asio::buffer(m.message));
          }
      }

      );
  }

~NetworkServer() override
{
  for (auto& t : this->_threads) {
    t.join();
  }
}


private:

asio::ip::tcp::socket accept_socket(asio::ip::tcp::acceptor &a) {
    asio::ip::tcp::socket s(this->_io_c);
    a.accept(s);
    LOGGER("server", LogLevel::DEBUG, "client accepted");
    return s;
}

void launch_server(ServerLaunching const &s) {
    std::cout << "prout\n";
    LOGGER("server", LogLevel::DEBUG, "thread started");
    asio::ip::tcp::acceptor a(_io_c, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), s.port));
    asio::ip::tcp::socket socket(this->_io_c);

    LOGGER("server", LogLevel::DEBUG, "try accept client");
    this->_clients.push_back(this->accept_socket(a));

    int i = 0;
    while (i < 10) {
        asio::write(this->_clients[0], asio::buffer("prout pipi"));
        i++;
    }

}

asio::io_context _io_c;
std::vector<std::thread> _threads;
std::vector<asio::ip::tcp::socket> _clients;
};
