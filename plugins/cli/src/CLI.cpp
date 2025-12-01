#include <cstdlib>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>

#include "CLI.hpp"

#include "ClientConnection.hpp"
#include "Events.hpp"
#include "ServerLaunch.hpp"
#include "plugin/APlugin.hpp"

CLI::CLI(Registery& r, EntityLoader& l)
    : APlugin(r,
              l,
              {"logger", "server_network", "client_network"},
              {COMP_INIT(CliComp, init_cli)})
{
  _registery.get().on<ShutdownEvent>([this](const ShutdownEvent& event)
                                     { _running = false; });

  _registery.get().on<CleanupEvent>([this](const CleanupEvent& event)
                                    { _running = false; });

  _registery.get().register_component<CliComp>("cli:CliComp");

  _registery.get().on<CliStart>(
      [this](const CliStart&)
      {
        if (!this->_running) {
          _cli_thread = std::thread(&CLI::run_cli, this);
        }
      });
}

CLI::~CLI()
{
  _running = false;
  if (_cli_thread.joinable()) {
    _cli_thread.join();
  }
}

void CLI::init_cli(Registery::Entity const, JsonVariant const&)
{
  _registery.get().emit<CliStart>();
}

void CLI::run_cli()
{
  std::string line;

  _running = true;
  while (_running) {
    std::cout << "> " << std::flush;

    if (!std::getline(std::cin, line)) {
      _registery.get().emit<ShutdownEvent>("Cli end", 0);
      break;
    }

    if (!_running) {
      _registery.get().emit<ShutdownEvent>("Error in cli", 0);
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

  const std::unordered_map<std::string,
                           std::function<void(std::istringstream&)>>
      COMMANDS = {
          {"help",
           [](std::istringstream&)
           {
             std::cout << "Available commands:\n"
                       << "  help                  - Show this help\n"
                       << "  log <msg>             - Emit LogEvent\n"
                       << "  server <port>         - Start server\n"
                       << "  connect <host> <port> - Connect to server\n"
                       << "  quit                  - Quit\n";
           }},
          {"log",
           [this](std::istringstream& iss)
           {
             std::string message;
             std::getline(iss, message);
             if (!message.empty() && message[0] == ' ') {
               message = message.substr(1);
             }
             _registery.get().emit<LogEvent>(
                 "cli",
                 LogLevel::INFO,
                 message.empty() ? "test message" : message);
           }},
          {"server",
           [this](std::istringstream& iss)
           {
             uint16_t port = 0;
             iss >> port;

             if (port == 0) {
               std::cout << "Usage: server <port>\n";
               return;
             }

             _registery.get().emit<ServerLaunching>(port);
             std::cout << "Starting server on " << "0.0.0.0" << ":" << port
                       << "\n";
           }},
          {"connect",
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

             _registery.get().emit<ClientConnection>(host, port);
             std::cout << "Connecting to " << host << ":" << port << "\n";
           }},
          {"shutdown",
           [this](std::istringstream& iss)
           {
             std::string reason;
             std::getline(iss, reason);
             if (!reason.empty() && reason[0] == ' ') {
               reason = reason.substr(1);
             }
             _registery.get().emit<ShutdownEvent>(
                 reason.empty() ? "CLI requested" : reason, 0);
           }},
          {"quit",
           [this](std::istringstream& iss)
           {
             std::string reason;
             std::getline(iss, reason);
             if (!reason.empty() && reason[0] == ' ') {
               reason = reason.substr(1);
             }
             _registery.get().emit<ShutdownEvent>(
                 reason.empty() ? "CLI requested" : reason, 0);
           }},
          {"cleanup",
           [this](std::istringstream& iss)
           {
             std::string trigger;
             std::getline(iss, trigger);
             if (!trigger.empty() && trigger[0] == ' ') {
               trigger = trigger.substr(1);
             }
             _registery.get().emit<CleanupEvent>(trigger.empty() ? "CLI"
                                                                 : trigger);
           }}};

  auto it = COMMANDS.find(command);
  if (it != COMMANDS.end()) {
    it->second(iss);
  } else {
    std::cout << "Unknown command: '" << command
              << "'. Type 'help' for available commands.\n";
  }
}

extern "C"
{
void* entry_point(Registery& r, EntityLoader& l)
{
  return new CLI(r, l);
}
}
