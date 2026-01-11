#include <format>
#include <stdexcept>
#include <variant>

#include "Json/JsonParser.hpp"
#include "ParserTypes.hpp"
#include "ecs/Registry.hpp"
#include "network/HttpClient.hpp"
#include "network/Httplib.hpp"
#include "network/client/BaseClient.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/events/HttpEvents.hpp"
#include "plugin/events/LogMacros.hpp"
#include "plugin/events/LoggerEvent.hpp"

void BaseClient::setup_http_requests()
{
  this->_registry.get().add_system(
      [this](Registry& /* */)
      { this->_http_client.handle_responses(static_cast<void*>(this)); });

  SUBSCRIBE_EVENT(FetchAvailableServers, { this->handle_server_fetch(); })
}

void handle_fetch_servers(void* raw_context, httplib::Result const& result)
{
  auto* context = static_cast<BaseClient*>(raw_context);
  auto parsed = parseJsonArray()(result->body);

  if (parsed.index() == ERROR) {
    CONTEXT_LOGGER(
        context,
        "http",
        LogLevel::ERROR,
        "failed to parse http response: " + std::get<ERROR>(parsed).message);
    return;
  }
  context->_available_servers.clear();
  JsonArray const& r = std::get<SUCCESS>(parsed).value;
  for (auto const& it : r) {
    try {
      auto const& obj = std::get<JsonObject>(it.value);
      auto const& id = std::get<int>(obj.at("id").value);
      auto const& host = std::get<std::string>(obj.at("address").value);
      auto const& port = std::get<int>(obj.at("port").value);
      context->_available_servers.emplace_back(id, host, port);
    } catch (std::bad_variant_access const&) {
      CONTEXT_LOGGER(context,
                     "http",
                     LogLevel::WARNING,
                     "wrong json type in resonse, skipping");
    } catch (std::out_of_range const&) {
      CONTEXT_LOGGER(context,
                     "http",
                     LogLevel::WARNING,
                     "wrong json type in resonse object, skipping");
    }
  }
  std::cout << "[";
  for (auto const& it : context->_available_servers) {
    std::cout << std::format(
        "[id: {}, host: {}, port {}]", it.id, it.address, it.port);
  }
  std::cout << "]\n";
}

void BaseClient::handle_server_fetch()
{
  this->_http_client.register_get(&handle_fetch_servers, "/active_server/" + this->game_name);
}
