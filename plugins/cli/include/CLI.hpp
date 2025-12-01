#pragma once

#include <atomic>
#include <string>
#include <thread>

#include "Events.hpp"
#include "ecs/Registery.hpp"
#include "plugin/APlugin.hpp"

class CLI : public APlugin
{
public:
  CLI(Registery& r, EntityLoader& l);
  ~CLI();

private:
  void run_cli();
  void process_command(const std::string& cmd);
  void init_cli(Registery::Entity const entity, JsonVariant const& config);

  std::thread _cli_thread;
  std::atomic<bool> _running = false;
};
