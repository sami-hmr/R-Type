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
  std::vector<ConnectedPackage> extract_available_packages();
  void register_sent_package(ConnectedPackage const&);
  std::vector<ByteArray> get_packages_to_send(std::vector<std::size_t> const &asked_packages);
  void approuve_packages(std::size_t acknowledge);

  //////////////////
  // RECEIVER
  //////////////////
  std::size_t get_acknowledge() const;
  std::vector<std::size_t> get_lost_packages();
  void register_received_package(ConnectedPackage const&);

  void reset(std::size_t sequence);
  void reset();
private:

  static const std::size_t sent_delta = 1000000; // 0.1 second;

  struct AwaitingPackage {
      ConnectedPackage package;
      std::size_t true_delta = 0;
      std::size_t send_delta = 0;
  };
  std::map<std::size_t, AwaitingPackage> _awaiting_packages;
  static const std::size_t ask_cooldown = 2000000; // 0.2 second;
  std::map<std::size_t, std::size_t> _asked_delta;
  std::map<std::size_t, AwaitingPackage> _waiting_for_aprouval;

  std::size_t _last_extracted = 0;
};
