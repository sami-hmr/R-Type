
#include "NetworkCommun.hpp"
#include "NetworkShared.hpp"
#include "Server.hpp"
#include "plugin/Byte.hpp"

void Server::transmit_event_to_client(EventBuilderId&& to_transmit)
{
  this->_events_to_transmit.get().lock.lock();
  this->_events_to_transmit.get().queue.push(std::move(to_transmit));
  this->_events_to_transmit.get().lock.unlock();
  this->_semaphore_event.get().release();
}

void Server::transmit_event_to_server(EventBuilder&& to_transmit)
{
  this->_events_queue.get().lock.lock();
  this->_events_queue.get().queue.push(std::move(to_transmit));
  this->_events_queue.get().lock.unlock();
  this->_semaphore.get().release();
}

void Server::send_event_to_client()
{
  while (this->_running) {
    this->_semaphore_event.get().acquire();
    this->_events_to_transmit.get().lock.lock();
    this->_client_mutex.lock();
    auto& queue = this->_events_to_transmit.get().queue;
    while (!queue.empty()) {
      auto const& evt = queue.front();
      ByteArray data = type_to_byte(SENDEVENT) + evt.event.to_bytes();
      if (evt.client) {
        auto const& client = this->find_client_by_id(*evt.client);
        this->send_connected(data, client.endpoint);
      } else {
        for (auto const& it : this->_clients) {
          if (it.state != ClientState::CONNECTED) {
            continue;
          }
          this->send_connected(data, it.endpoint);
        }
      }
      queue.pop();
    }
    this->_events_to_transmit.get().lock.unlock();
    this->_client_mutex.unlock();
  }
}

void Server::send_comp()
{
  while (this->_running) {
    this->_semaphore.get().acquire();
    this->_components_to_create.get().lock.lock();
    this->_client_mutex.lock();
    while (!this->_components_to_create.get().queue.empty()) {
      auto const& comp = this->_components_to_create.get().queue.front();
      ByteArray data = type_to_byte<std::uint8_t>(SENDCOMP)
          + type_to_byte(comp.component.entity)
          + string_to_byte(comp.component.id) + comp.component.data;
      if (comp.client) {
        this->send_connected(
            data, this->find_client_by_id(comp.client.value()).endpoint);
      } else {
        for (auto const& it : this->_clients) {
          if (it.state != ClientState::CONNECTED) {
            continue;
          }
          this->send_connected(data, it.endpoint);
        }
      }
      this->_components_to_create.get().queue.pop();
    }
    this->_client_mutex.unlock();
    this->_components_to_create.get().lock.unlock();
  }
}
