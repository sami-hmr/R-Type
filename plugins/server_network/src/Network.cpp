#include <cstring>
#include <format>
#include <functional>
#include <thread>

#include "Network.hpp"
#include "Server.hpp"

#include "ecs/Registry.hpp"
#include "plugin/events/Log.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/events/Events.hpp"
#include "plugin/events/CleanupEvent.hpp"
#include "plugin/events/ShutdownEvent.hpp"

NetworkServer::NetworkServer(Registry& r, EntityLoader& l)
    : APlugin(r, l, {}, {})
{

  this->_registry.get().on<ServerLaunching>(
    [this](ServerLaunching const& s)
  {
    this->_threads.emplace_back([this, s]() { this->launch_server(s); });
  });

  this->_registry.get().on<ShutdownEvent>(
    [this](ShutdownEvent const& event)
  {
    _running = false;
    LOGGER("server", LogLevel::INFO,
      std::format("Shutdown requested: {}", event.reason));
    // server.close();
  });

  this->_registry.get().on<CleanupEvent>(
    [this](CleanupEvent const&)
  {
    _running = false;
    LOGGER("server", LogLevel::DEBUG, "Cleanup requested");
    // server.close();
  });
}

NetworkServer::~NetworkServer()
{
  _running = false;
  for (auto& t : this->_threads) {
    t.join();
  }
}

void NetworkServer::launch_server(ServerLaunching const& s)
{
  try {
    _running = true;
    Server server(s, _components_to_update, _running);
    LOGGER("server",
           LogLevel::INFO,
           std::format("Server started on port {}", s.port));

      server.receive_loop();
  } catch (std::exception& e) {
    LOGGER("server",
           LogLevel::ERROR,
           std::format("Failed to start server: {}", e.what()));
  }
}

// asio::socket_base::message_flags Server::handle_receive(
//     const asio::error_code& UNUSED error, std::size_t UNUSED bytes_transferred)
// {
//   asio::socket_base::message_flags flag = MSG_OOB;
//   std::cout << "handled the reception" << std::endl;
//   return flag;
// }

extern "C"
{
void* entry_point(Registry& r, EntityLoader& e)
{
  return new NetworkServer(r, e);
}
}
