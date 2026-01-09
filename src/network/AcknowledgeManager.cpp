#include <chrono>
#include <stdexcept>
#include <vector>

#include "network/AcknowledgeManager.hpp"

#include "NetworkCommun.hpp"
#include "ServerCommands.hpp"
#include "plugin/Byte.hpp"

void AcknowledgeManager::register_sent_package(ConnectedPackage const& pkg)
{
  std::size_t const& now =
      std::chrono::steady_clock::now().time_since_epoch().count();
  this->_waiting_for_aprouval[pkg.sequence_number] =
      AwaitingPackage(pkg, now, now);
}

std::vector<ConnectedPackage> AcknowledgeManager::extract_available_packages()
{
  std::vector<ConnectedPackage> result;

  for (auto const& [sequence, pkg] : this->_awaiting_packages) {
    if (sequence != (this->_last_extracted + 1)) {
      break;
    }
    result.push_back(pkg.package);
    this->_last_extracted = sequence;
  }
  this->_awaiting_packages.erase(
      this->_awaiting_packages.begin(),
      this->_awaiting_packages.upper_bound(this->_last_extracted));
  this->_asked_delta.erase(
      this->_asked_delta.begin(),
      this->_asked_delta.upper_bound(this->_last_extracted));

  return result;
}

std::vector<ByteArray> AcknowledgeManager::get_packages_to_send(
    std::vector<std::size_t> const& asked_packages)
{
  std::size_t now = std::chrono::steady_clock::now().time_since_epoch().count();

  std::vector<ByteArray> result;

  for (auto const& it : asked_packages) {
    try {
      auto& package = this->_waiting_for_aprouval.at(it);
      if ((package.send_delta) <= now) {
        package.package.acknowledge = this->get_acknowledge();
        result.push_back(package.package.to_bytes());
        package.send_delta = now + AcknowledgeManager::sent_delta;
      }
    } catch (std::out_of_range const&) {
      LOGGER_EVTLESS(
          LogLevel::WARNING,
          "acknowledge",
          std::format("Package {} does not exist or has been approuved already",
                      it));
    }
  }
  return result;
}

void AcknowledgeManager::approuve_packages(std::size_t acknowledge)
{
  this->_waiting_for_aprouval.erase(
      this->_waiting_for_aprouval.begin(),
      this->_waiting_for_aprouval.upper_bound(acknowledge));
}

///////////////////////
// RECEIVER
///////////////////////

void AcknowledgeManager::register_received_package(ConnectedPackage const& pkg)
{
  if (pkg.sequence_number <= this->_last_extracted) {
    return;
  }
  std::size_t const& now =
      std::chrono::steady_clock::now().time_since_epoch().count();
  this->_awaiting_packages[pkg.sequence_number] =
      AwaitingPackage(pkg, now, now);
}

std::vector<std::size_t> AcknowledgeManager::get_lost_packages()
{
  std::size_t last_package = this->_last_extracted;
  std::vector<std::size_t> result;
  std::size_t const& now =
      std::chrono::steady_clock::now().time_since_epoch().count();

  for (auto const& it : this->_awaiting_packages) {
    if (it.first != last_package + 1) {
      for (std::size_t i = last_package + 1; i < it.first; i++) {
        if (now - this->_asked_delta[i] > ask_cooldown) {
          result.push_back(i);
          this->_asked_delta[i] = now;
        }
      }
    }
    last_package = it.first;
  }
  // for (auto const &it : result) {
  //     std::cout << it << ", ";
  // }
  // std::cout << std::endl;
  return result;
}

std::size_t AcknowledgeManager::get_acknowledge() const
{
  return this->_last_extracted;
}

void AcknowledgeManager::reset()
{
  if (!this->_awaiting_packages.empty()) {
    this->_last_extracted = this->_awaiting_packages.crbegin()->first;
    // std::cout << "LAST EXTRACTED " << this->_last_extracted << std::endl;
  }
  // this->_waiting_for_aprouval.clear();
}

void AcknowledgeManager::reset(std::size_t sequence)
{
  this->_last_extracted = sequence;
  // this->_waiting_for_aprouval.clear();
}
