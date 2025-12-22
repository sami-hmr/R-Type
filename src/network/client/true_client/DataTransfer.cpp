#include "NetworkCommun.hpp"
#include "NetworkShared.hpp"
#include "network/client/Client.hpp"
#include "plugin/Byte.hpp"

void Client::transmit_component(ComponentBuilder&& to_transmit)
{
  this->_components_to_create.get().lock.lock();
  this->_components_to_create.get().queue.push(std::move(to_transmit));
  this->_components_to_create.get().lock.unlock();
}

void Client::transmit_event(EventBuilder&& to_transmit)
{
  this->_event_to_exec.get().lock.lock();
  this->_event_to_exec.get().queue.push(std::move(to_transmit));
  this->_event_to_exec.get().lock.unlock();
}

void Client::send_evt()
{
  while (_running.get()) {
    this->_events_to_transmit.get().wait();
    auto const& events = this->_events_to_transmit.get().flush();

    for (auto const& evt : events) {
      auto data = type_to_byte<Byte>(SENDEVENT) + evt.to_bytes();
      this->send_connected(data);
    }
  }
}
