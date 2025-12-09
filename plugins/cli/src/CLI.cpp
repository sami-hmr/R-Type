#include <cstdlib>
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <unordered_map>

#include "CLI.hpp"

#include "ClientConnection.hpp"
#include "NetworkShared.hpp"
#include "ServerLaunch.hpp"
#include "ecs/Scenes.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/components/Drawable.hpp"
#include "plugin/components/Position.hpp"
#include "plugin/components/Sprite.hpp"
#include "plugin/events/Events.hpp"

CLI::CLI(Registry& r, EntityLoader& l, std::optional<JsonObject> const& config)
    : APlugin(r, l, {"logger", "server_network", "client_network"}, {}, config)
{
  _registry.get().on<ShutdownEvent>([this](const ShutdownEvent&)
                                     { _running = false; });

  _registry.get().on<CleanupEvent>([this](const CleanupEvent&)
                                    { _running = false; });

  _registry.get().on<CliStart>(
      [this](const CliStart&)
      {
        if (!this->_running) {
          _cli_thread = std::thread(&CLI::run_cli, this);
        }
      });

  _registry.get().on<CliStop>([this](const CliStop&) { _running = false; });

  _registry.get().emit<CliStart>();
}

CLI::~CLI()
{
  _running = false;
  if (_cli_thread.joinable()) {
    _cli_thread.join();
  }
}

void CLI::run_cli()
{
  std::string line;

  _running = true;
  while (_running) {
    std::cout << "> " << std::flush;

    if (!std::getline(std::cin, line)) {
     // _registry.get().emit<ShutdownEvent>("Cli end", 0);
      break;
    }

    if (!_running) {
     // _registry.get().emit<ShutdownEvent>("Error in cli", 0);
      break;
    }

    if (line.empty()) {
      continue;
    }

    process_command(line);
  }
  exit(0);
}

void CLI::process_command(const std::string& cmd)
{
  std::istringstream iss(cmd);
  std::string command;
  iss >> command;

  struct Command
  {
    std::string usage;
    std::string description;
    std::function<void(std::istringstream&)> handler;
  };

  const std::map<std::string, Command> COMMANDS = {
      {"help",
       {.usage = "help",
        .description = "Show this help",
        .handler =
            [&COMMANDS](std::istringstream&)
        {
          std::cout << "Available commands:\n";
          for (const auto& [name, cmd] : COMMANDS) {
            std::cout << "  " << std::left << std::setw(22) << cmd.usage
                      << " - " << cmd.description << "\n";
          }
        }}},
      {"log",
       {.usage = "log <msg>",
        .description = "Emit LogEvent",
        .handler =
            [this](std::istringstream& iss)
        {
          std::string message;
          std::getline(iss, message);
          if (!message.empty() && message[0] == ' ') {
            message = message.substr(1);
          }
          _registry.get().emit<LogEvent>(
              "cli",
              LogLevel::INFO,
              message.empty() ? "test message" : message);
        }}},
      {"server",
       {.usage = "server <port>",
        .description = "Start server",
        .handler =
            [this](std::istringstream& iss)
        {
          uint16_t port = 0;
          iss >> port;

          if (port == 0) {
            std::cout << "Usage: server <port>\n";
            return;
          }

          _registry.get().emit<ServerLaunching>(port);
          std::cout << "Starting server on "
                    << "0.0.0.0"
                    << ":" << port << "\n";
        }}},
      {"connect",
       {.usage = "connect <host> <port>",
        .description = "Connect to server",
        .handler =
            [this](std::istringstream& iss)
        {
          std::string host;
          uint16_t port = 0;
          iss >> host >> port;

          if (host.empty() || port == 0) {
            std::cout << "Usage: connect <host> <port>\n";
            std::cout << "Example: connect 127.0.0.1 27015\n";
            return;
          }
          _registry.get().emit<ClientConnection>(host, port);
          std::cout << "Connecting to " << host << ":" << port << "\n";
        }}},
      {"s",
       {.usage = "s",
        .description = "commande de goat pour lancer le server",
        .handler =
            [this](std::istringstream&)
        {
          uint16_t port = 4242;
          _registry.get().emit<ServerLaunching>(port);
          std::cout << "Starting server on "
                    << "0.0.0.0"
                    << ":" << port << "\n";
        }}},
      {"c",
       {.usage = "c",
        .description = "autre commande de goat pour connect le client",
        .handler =
            [this](std::istringstream&)
        {
          std::string host = "0.0.0.0";
          uint16_t port = 4242;
          if (host.empty() || port == 0) {
            std::cout << "Usage: connect <host> <port>\n";
            std::cout << "Example: connect 127.0.0.1 27015\n";
            return;
          }
          _registry.get().emit<ClientConnection>(host, port);
          std::cout << "Connecting to " << host << ":" << port << "\n";
        }}},
      {"spawn",
       {.usage = "spawn",
        .description = "spawn entity with drawing de con en 0, 0",
        .handler =
            [this](std::istringstream&)
        {
            Drawable draw;
            Sprite sprite("ça existe meme pas", {1, 1});
            Position pos(0, 0);
            Scene scene("game", SceneState::ACTIVE);
            this->_registry.get().emit<ComponentBuilder>(42, "sfml:Drawable", draw.to_bytes());
            this->_registry.get().emit<ComponentBuilder>(42, "sfml:Sprite", sprite.to_bytes());
            this->_registry.get().emit<ComponentBuilder>(42, "moving:Position", pos.to_bytes());
            this->_registry.get().emit<ComponentBuilder>(42, "scene", scene.to_bytes());
        }}},

      {"stop",
       {.usage = "stop",
        .description = "Stop CLI thread",
        .handler =
            [this](std::istringstream&)
        {
          std::cout << "Stopping CLI...\n";
          _registry.get().emit<CliStop>();
        }}},
      {"quit",
       {.usage = "quit [reason]",
        .description = "Quit the application",
        .handler =
            [this](std::istringstream& iss)
        {
          std::string reason;
          std::getline(iss, reason);
          if (!reason.empty() && reason[0] == ' ') {
            reason = reason.substr(1);
          }
          _registry.get().emit<ShutdownEvent>(
              reason.empty() ? "CLI requested" : reason, 0);
        }}},
      {"cleanup",
       {.usage = "cleanup [trigger]",
        .description = "Emit cleanup event",
        .handler = [this](std::istringstream& iss)
        {
          std::string trigger;
          std::getline(iss, trigger);
          if (!trigger.empty() && trigger[0] == ' ') {
            trigger = trigger.substr(1);
          }
          _registry.get().emit<CleanupEvent>(trigger.empty() ? "CLI"
                                                              : trigger);
        }}}};

  auto it = COMMANDS.find(command);
  if (it != COMMANDS.end()) {
    it->second.handler(iss);
  } else {
    std::cout << "Unknown command: '" << command
              << "'. Type 'help' for available commands.\n";
  }
}

extern "C"
{
void* entry_point(Registry& r,
                  EntityLoader& l,
                  std::optional<JsonObject> const& config)
{
  return new CLI(r, l, config);
}
}
