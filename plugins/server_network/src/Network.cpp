#include <chrono>
#include <format>
#include <functional>
#include <thread>

#include "Network.hpp"

#include <unistd.h>

#include "NetworkShared.hpp"
#include "Server.hpp"
#include "ecs/InitComponent.hpp"
#include "ecs/Registry.hpp"
#include "ecs/Scenes.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/components/AnimatedSprite.hpp"
#include "plugin/components/Collidable.hpp"
#include "plugin/components/Drawable.hpp"
#include "plugin/components/Health.hpp"
#include "plugin/components/Position.hpp"
#include "plugin/components/Sprite.hpp"
#include "plugin/components/Team.hpp"
#include "plugin/components/Velocity.hpp"
#include "plugin/events/CleanupEvent.hpp"
#include "plugin/events/LoggerEvent.hpp"
#include "plugin/events/ShutdownEvent.hpp"

NetworkServer::NetworkServer(Registry& r, EntityLoader& l)
    : APlugin(r, l, {}, {})
    , _comp_semaphore(0)
    , _event_semaphore(0)
{
  this->_registry.get().on<ServerLaunching>(
      "ServerLaunching",
      [this](ServerLaunching const& s)
      {
        this->_thread = std::thread([this, s]() { this->launch_server(s); });
      });

  this->_registry.get().on<ShutdownEvent>(
      "ShutdownEvent",
      [this](ShutdownEvent const& event)
      {
        _running = false;
        LOGGER("server",
               LogLevel::INFO,
               std::format("Shutdown requested: {}", event.reason));
        // server.close();
      });

  this->_registry.get().on<CleanupEvent>(
      "CleanupEvent",
      [this](CleanupEvent const&)
      {
        _running = false;
        LOGGER("server", LogLevel::DEBUG, "Cleanup requested");
        // server.close();
      });

  this->_registry.get().on<ComponentBuilder>(
      "ComponentBuilder",
      [this](ComponentBuilder e)
      {
        this->_components_to_update.lock.lock();
        this->_components_to_update.queue.push(std::move(e));
        this->_components_to_update.lock.unlock();
        this->_comp_semaphore.release();
      });

  this->_registry.get().add_system<>(
      [this](Registry& r)
      {
        this->_event_queue.lock.lock();
        while (!this->_event_queue.queue.empty()) {
          auto& e = this->_event_queue.queue.front();
          r.emit(e.event_id, e.data);
          this->_event_queue.queue.pop();
        }
        this->_event_queue.lock.unlock();
      });

  this->_registry.get().on<EntityCreation>(
      "EntityCreation",
      [this](EntityCreation const& e)
      {
        std::size_t entity = this->_registry.get().spawn_entity();

        this->_event_queue_to_client.lock.lock();
        this->_event_queue_to_client.queue.emplace(
            e.client, "PlayerCreation", PlayerCreation(entity).to_bytes());
        this->_event_queue_to_client.lock.unlock();
        this->_event_semaphore.release();
      });

  this->_registry.get().on<PlayerCreated>(
      "PlayerCreated",
      [this](PlayerCreated const& data)
      {
        init_component(
            this->_registry.get(), data.server_index, Position(0, 0, 2));
        init_component(this->_registry.get(), data.server_index, Drawable());
        init_component(this->_registry.get(),
                       data.server_index,
                       Velocity(0.01, 0.01, 0, 0));
        init_component(this->_registry.get(),
                       data.server_index,
                       AnimatedSprite({{"idle",
                                        AnimationData("assets/player.png",
                                                      {350., 150.},
                                                      {0., 0.},
                                                      {1., 0.},
                                                      {0.2, 0.2},
                                                      10,
                                                      7,
                                                      0,
                                                      true,
                                                      true)}},
                                      "idle",
                                      "idle"));
        init_component(this->_registry.get(),
                       data.server_index,
                       Collidable(0.02, 0.02, CollisionType::Solid));
        init_component(
            this->_registry.get(), data.server_index, Health(5, 100));
        init_component(this->_registry.get(), data.server_index, Team("test1"));
        init_component(this->_registry.get(),
                       data.server_index,
                       Scene("game", SceneState::ACTIVE));
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
                  _components_to_update,
                  _event_queue_to_client,
                  _event_queue,
                  _running,
                  _comp_semaphore,
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
