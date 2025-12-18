#pragma once

#include <atomic>
#include <optional>
#include <string>
#include <thread>

#include "Json/JsonParser.hpp"
#include "ecs/Registry.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"

class CLI : public APlugin
{
public:
  CLI(Registry& r, EntityLoader& l, std::optional<JsonObject> const& config);
  ~CLI();

private:
  void run_cli();
  void process_command(const std::string& cmd);

  std::thread _cli_thread;
  std::atomic<bool> _running = false;
};
