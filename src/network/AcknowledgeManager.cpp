#include <chrono>

#include "network/AcknowledgeManager.hpp"

#include "ServerCommands.hpp"
#include "plugin/Byte.hpp"

void AcknowledgeManager::register_received_package(ConnectedPackage const& pkg)
{
    //std::cout << "received sequence : " << pkg.sequence_number << "  last extracted :" <<
  if (pkg.sequence_number <= this->_last_extracted) {
    return;
  }
  std::size_t const &now = std::chrono::steady_clock::now().time_since_epoch().count();
  this->_awaiting_packages[pkg.sequence_number] = AwaitingPackage(pkg, now, now);
}

void AcknowledgeManager::register_sent_package(ConnectedPackage const& pkg)
{
  std::size_t const &now = std::chrono::steady_clock::now().time_since_epoch().count();
  this->_waiting_for_aprouval[pkg.sequence_number] = AwaitingPackage(pkg, now, now);
}

std::vector<ConnectedPackage> AcknowledgeManager::extract_available_packages()
{
  std::vector<ConnectedPackage> result;
  std::size_t const &now = std::chrono::steady_clock::now().time_since_epoch().count();

  for (auto const& [sequence, pkg] : this->_awaiting_packages) {
    if (sequence != (this->_last_extracted + 1) && (now - pkg.true_delta) < 500000000) {
      break;
    }
    result.push_back(pkg.package);
    this->_last_extracted = sequence;
  }
  this->_awaiting_packages.erase(
      this->_awaiting_packages.begin(),
      this->_awaiting_packages.upper_bound(this->_last_extracted));
  return result;
}

std::vector<ByteArray> AcknowledgeManager::get_packages_to_send()
{
  auto now = std::chrono::steady_clock::now().time_since_epoch().count();

  std::erase_if(this->_waiting_for_aprouval,
                [now](auto const& pair)
                {
                  bool should_timeout = (now - pair.second.true_delta) > 15000000000;/* 0.5 second */
                  if (should_timeout) {
                      std::cout << "timeout\n";
                  }
                  return should_timeout;
                });

  std::vector<ByteArray> result;
  result.reserve(this->_waiting_for_aprouval.size());

  for (auto &[id, it] : this->_waiting_for_aprouval) {
    if ((now - it.send_delta) > 300000000) {
        it.package.acknowledge = this->get_acknowledge();
        result.push_back(it.package.to_bytes());
        it.send_delta = now;
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

std::size_t AcknowledgeManager::get_acknowledge() const
{
  return this->_last_extracted;
}
