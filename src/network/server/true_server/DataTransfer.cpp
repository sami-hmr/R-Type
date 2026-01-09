
#include <iostream>

#include <asio/registered_buffer.hpp>

#include "NetworkCommun.hpp"
#include "NetworkShared.hpp"
#include "network/server/Server.hpp"
#include "plugin/Byte.hpp"

void Server::transmit_event_to_client(EventBuilderId const& to_transmit)
{
  this->_events_queue_to_client.get().push(to_transmit);
}

void Server::transmit_event_to_server(EventBuilder const& to_transmit)
{
  this->_events_queue_to_serv.get().push(to_transmit);
}

void Server::send_event_to_client()
{
  while (this->_running) {
    this->_events_queue_to_client.get().wait();
    auto events = this->_events_queue_to_client.get().flush();
    this->_client_mutex.lock();
    for (auto const& evt : events) {
      ByteArray data = type_to_byte(SENDEVENT) + evt.event.to_bytes();
      if (evt.client) {
        try {
          auto& client = this->find_client_by_id(*evt.client);
          this->send_connected(data, client);
        } catch (ClientNotFound const& e) {
          LOGGER_EVTLESS(
              LogLevel::WARNING,
              "server",
              std::format("Cannot send event to client: {} (context: {})",
                          e.what(),
                          e.format_context()));
        }
      } else {
        for (auto& it : this->_clients) {
          if (it.state != ClientState::CONNECTED) {
            continue;
          }
          this->send_connected(data, it);
        }
      }
    }
    this->_client_mutex.unlock();
  }
}

void Server::send_comp()
{
  while (this->_running) {
    this->_components_to_create.get().wait();
    auto components = this->_components_to_create.get().flush();
    this->_client_mutex.lock();
    for (auto const& comp : components) {
      ByteArray data = type_to_byte<std::uint8_t>(SENDCOMP)
          + type_to_byte(comp.component.entity)
          + string_to_byte(comp.component.id) + comp.component.data;
      if (comp.client) {
        try {
          this->send_connected(data,
                               this->find_client_by_id(comp.client.value()));
        } catch (ClientNotFound const& e) {
          LOGGER_EVTLESS(
              LogLevel::WARNING,
              "server",
              std::format("Cannot send component to client: {} (context: {})",
                          e.what(),
                          e.format_context()));
        }
      } else {
        for (auto& it : this->_clients) {
          if (it.state != ClientState::CONNECTED) {
            continue;
          }
          this->send_connected(data, it);
        }
      }
    }
    this->_client_mutex.unlock();
  }
}
