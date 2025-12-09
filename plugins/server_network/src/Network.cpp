#include <format>
#include <functional>
#include <thread>

#include "Network.hpp"

#include "NetworkShared.hpp"
#include "Server.hpp"
#include "ecs/Registry.hpp"
#include "plugin/events/LoggerEvent.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/events/Events.hpp"
#include "plugin/events/CleanupEvent.hpp"
#include "plugin/events/ShutdownEvent.hpp"
#include "plugin/events/LoggerEvent.hpp"

NetworkServer::NetworkServer(Registry& r, EntityLoader& l)
    : APlugin(r, l, {}, {})
    , _event_semaphore(0)
{
<<<<<<< Updated upstream
  this->_registry.get().on<ServerLaunching>("ServerLaunching",
=======
  this->_registry.get().on<ServerLaunching>(
      "ServerLaunching",
>>>>>>> Stashed changes
      [this](ServerLaunching const& s)
      {
        this->_thread = std::thread([this, s]() { this->launch_server(s); });
      });

<<<<<<< Updated upstream
  this->_registry.get().on<ShutdownEvent>("ShutdownEvent", [this](ShutdownEvent const& event)
=======
  this->_registry.get().on<ShutdownEvent>(
      "ShutdownEvent",
      [this](ShutdownEvent const& event)
>>>>>>> Stashed changes
      {
        _running = false;
        LOGGER("server",
               LogLevel::INFO,
               std::format("Shutdown requested: {}", event.reason));
        // server.close();
      });

<<<<<<< Updated upstream
  this->_registry.get().on<CleanupEvent>("CleanupEvent", [this](CleanupEvent const&)
=======
  this->_registry.get().on<CleanupEvent>(
      "CleanupEvent",
      [this](CleanupEvent const&)
>>>>>>> Stashed changes
      {
        _running = false;
        LOGGER("server", LogLevel::DEBUG, "Cleanup requested");
        // server.close();
      });

<<<<<<< Updated upstream
  this->_registry.get().on<ComponentBuilder>("ComponentBuilder", [this](ComponentBuilder e)
=======
  this->_registry.get().on<ComponentBuilder>(
      "ComponentBuilder",
      [this](ComponentBuilder e)
>>>>>>> Stashed changes
      {
        this->_components_to_update.lock.lock();
        this->_components_to_update.queue.push(std::move(e));
        this->_components_to_update.lock.unlock();
        this->_event_semaphore.release();
      });

  this->_registry.get().add_system<>(
      [this](Registry& r)
      {
        ComponentBuilder t;

        this->_event_queue.lock.lock();
        while (!this->_event_queue.queue.empty()) {
          auto& e = this->_event_queue.queue.front();
          std::cout << e.event_id << " emmited" << std::endl;
          // r.emit(std::move(e.event_id), std::move(e.data));
          this->_event_queue.queue.pop();
        }
        this->_event_queue.lock.unlock();
      });
}

NetworkServer::~NetworkServer()
{
  _running = false;
  if (this->_thread.joinable()) {
    this->_thread.join();
  }
}

void NetworkServer::launch_server(ServerLaunching const& s)
{
  try {
    _running = true;
    Server server(s,
                  this->_components_to_update,
                  _event_queue,
                  _running,
                  _event_semaphore);
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
//     const asio::error_code& UNUSED error, std::size_t UNUSED
//     bytes_transferred)
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
