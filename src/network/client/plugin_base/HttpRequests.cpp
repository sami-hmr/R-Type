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
  this->_registry.get().add_system([this](Registry& /* */)
                                   { this->_http_client.handle_responses(); });

  SUBSCRIBE_EVENT(FetchAvailableServers, { this->handle_server_fetch(); })
  SUBSCRIBE_EVENT(Register, { this->handle_register(event); })
  SUBSCRIBE_EVENT(Login, { this->handle_login(event); })
}

void handle_fetch_servers(void* raw_context, httplib::Result const& result)
{
  auto* context = static_cast<BaseClient*>(raw_context);
  auto parsed = parseJsonArray()(result->body);

  if (parsed.index() == ERR) {
    CONTEXT_LOGGER(
        context,
        "http",
        LogLevel::ERROR,
        ("failed to parse http response: " + std::get<ERR>(parsed).message));
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
}

void BaseClient::handle_server_fetch()
{
  this->_http_client.register_get(
      &handle_fetch_servers, this, "/active_server/" + this->game_name);
}

void handle_login_response(void* raw_context, httplib::Result const& result)
{
  auto* context = static_cast<BaseClient*>(raw_context);

  auto parsed = parseJsonObject()(result->body);

  if (parsed.index() == ERR) {
    CONTEXT_LOGGER(
        context,
        "http",
        LogLevel::ERROR,
        "failed to parse http response: " + std::get<ERR>(parsed).message);
    return;
  }

  auto const& obj = std::get<SUCCESS>(parsed).value;
  try {
    auto const& id = std::get<int>(obj.at("id").value);
    context->_user_id = id;
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

void BaseClient::handle_register(Register const& r)
{
  this->_http_client.register_post(
      &handle_login_response,
      this,
      "/register",
      std::format(R"({}"identifier":"{}","password":"{}"{})",
                  '{',
                  r.identifier,
                  r.password,
                  '}'));
}

void BaseClient::handle_login(Login const& r)
{
  this->_http_client.register_post(
      &handle_login_response,
      this,
      "/login",
      std::format(R"({}"identifier":"{}","password":"{}"{})",
                  '{',
                  r.identifier,
                  r.password,
                  '}'));
}
