#include "Client.hpp"
#include "NetworkShared.hpp"

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


void Client::send_evt() {

}
