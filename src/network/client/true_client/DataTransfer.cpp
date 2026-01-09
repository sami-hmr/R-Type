#include <cstddef>
#include <numeric>
#include <thread>
#include <vector>

#include "NetworkCommun.hpp"
#include "NetworkShared.hpp"
#include "network/client/Client.hpp"
#include "plugin/Byte.hpp"
#include "plugin/events/NetworkEvents.hpp"

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

static NetworkStatus::PacketLossLevel get_packet_loss_level(
    std::vector<std::size_t> const& it, std::size_t begin, std::size_t end)
{
  std::size_t count = std::accumulate(it.begin(), it.end(), 0);
  std::size_t total = end - begin;
  if (total == 0) {
    return NetworkStatus::NONE;
  }
  std::size_t percent = (count * 100) / total;

  if (percent > 20) {
    return NetworkStatus::HIGH;
  }
  if (percent > 10) {
    return NetworkStatus::MEDIUM;
  }
  if (percent > 0) {
    return NetworkStatus::LOW;
  }
  return NetworkStatus::NONE;
}

void Client::send_hearthbeat()
{
  std::size_t delta =
      std::chrono::steady_clock::now().time_since_epoch().count()
      + hearthbeat_delta;
  std::size_t rapport_delta =
      std::chrono::steady_clock::now().time_since_epoch().count()
      + rapport_cooldown;
  std::size_t package_begin = 0;
  std::vector<std::size_t> lost_sizes;

  while (_running.get()) {
    while (delta > static_cast<std::size_t>(
               std::chrono::steady_clock::now().time_since_epoch().count()))
    {
      std::this_thread::yield();
    }
    delta += Client::hearthbeat_delta;
    this->_acknowledge_mutex.lock();
    auto lost_packages = this->_acknowledge_manager.get_lost_packages();
    lost_sizes.push_back(lost_packages.size());
    this->_acknowledge_mutex.unlock();
    auto now = std::chrono::steady_clock::now().time_since_epoch().count();
    this->send(HearthBeat(now, lost_packages).to_bytes(), true);
    if (this->_running.get() && this->should_disconnect()) {
      this->transmit_event(
          EventBuilder("Disconnection", Disconnection().to_bytes()));
    }
    if (rapport_delta < static_cast<std::size_t>(
            std::chrono::steady_clock::now().time_since_epoch().count()))
    {
      this->_latency_mutex.lock();
      if (this->_latencies.size() != 0) {
        std::size_t sum = std::accumulate(
            this->_latencies.begin(), this->_latencies.end(), std::size_t(0));
        this->_acknowledge_mutex.lock();
        std::size_t package_end =
            this->_acknowledge_manager.get_last_received();
        this->_acknowledge_mutex.unlock();
        this->transmit_event(EventBuilder(
            "NetworkStatus",
            NetworkStatus(
                ((sum / this->_latencies.size()) / 1000000),
                get_packet_loss_level(lost_sizes, package_begin, package_end))
                .to_bytes()));
        this->_latencies.clear();
        this->_latency_mutex.unlock();
        lost_sizes.clear();
        package_begin = package_end;
      }
      rapport_delta += rapport_cooldown;
    }
  }
  this->_socket.send_to(asio::buffer(""), _client_endpoint);
}
