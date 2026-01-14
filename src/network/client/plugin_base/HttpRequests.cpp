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
      {
        try {
          this->_http_client.handle_responses();
        } catch (HttpBadCode const& e) {
          this->_event_manager.get().emit<HttpBadCodeEvent>(e.code, e.what());
        }
      });

  SUBSCRIBE_EVENT(FetchAvailableServers, { this->handle_server_fetch(); })
  SUBSCRIBE_EVENT(Register, { this->handle_register(event); })
  SUBSCRIBE_EVENT(Login, { this->handle_login(event); })
}

void handle_fetch_servers(void* raw_context, httplib::Result const& result)
{
  auto* context = static_cast<BaseClient*>(raw_context);
  auto parsed =
      PARSE_HTTP_BODY((result->body), context, parseJsonArray, JsonArray);

  if (!parsed) {
    return;
  }
  context->_available_servers.clear();
  for (auto const& it : *parsed) {
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

  auto parsed =
      PARSE_HTTP_BODY((result->body), context, parseJsonObject, JsonObject);

  if (!parsed) {
    context->_event_manager.get().emit<FailLogin>();
    return;
  }

  auto const& obj = *parsed;
  try {
    auto const& id = std::get<int>(obj.at("id").value);
    context->_user_id = id;
    context->_event_manager.get().emit<LoginSuccessfull>(id);
    return;
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
  context->_event_manager.get().emit<FailLogin>();
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
