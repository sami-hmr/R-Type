
#include "NetworkCommun.hpp"
#include "NetworkShared.hpp"
#include "Server.hpp"
#include "plugin/Byte.hpp"

void Server::transmit_event(EventBuilder&& to_transmit)
{
  this->_events_to_transmit.get().lock.lock();
  this->_events_to_transmit.get().queue.push(std::move(to_transmit));
  this->_events_to_transmit.get().lock.unlock();
}

void Server::send_comp()
{
  while (this->_running) {
    this->_semaphore.get().acquire();
    this->_components_to_create.get().lock.lock();
    this->_client_mutex.lock();
    while (!this->_components_to_create.get().queue.empty()) {
      auto const& comp = this->_components_to_create.get().queue.front();
      for (auto const& it : this->_clients) {
        if (it.state != ClientState::CONNECTED) {
          continue;
        }
        this->send_connected(type_to_byte<std::uint8_t>(SENDCOMP)
                                 + type_to_byte(comp.entity)
                                 + string_to_byte(comp.id) + comp.data,
                             it.endpoint);
      }
      this->_components_to_create.get().queue.pop();
    }
    this->_client_mutex.unlock();
    this->_components_to_create.get().lock.unlock();
  }
}
