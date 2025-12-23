#pragma once

#include <chrono>
#include <cstddef>
#include <map>
#include <vector>

#include "ServerCommands.hpp"
#include "plugin/Byte.hpp"

class AcknowledgeManager
{
public:
  void register_received_package(ConnectedPackage const&);
  std::vector<ConnectedPackage> extract_available_packages();
  std::size_t get_acknowledge() const;

  void register_sent_package(ConnectedPackage const&);
  std::vector<ByteArray> get_packages_to_send();
  void approuve_packages(std::size_t acknowledge);

private:

  struct AwaitingPackage {
      ConnectedPackage package;
      std::size_t true_delta = 0;
      std::size_t send_delta = 0;
  };
  std::map<std::size_t, AwaitingPackage> _awaiting_packages;
  std::map<std::size_t, AwaitingPackage> _waiting_for_aprouval;

  std::size_t _last_extracted = 0;
};
