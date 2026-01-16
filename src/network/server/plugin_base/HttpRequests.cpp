#include <variant>

#include "Json/JsonParser.hpp"
#include "ParserTypes.hpp"
#include "ServerLaunch.hpp"
#include "network/HttpClient.hpp"
#include "network/Httplib.hpp"
#include "network/server/BaseServer.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/events/HttpEvents.hpp"
#include "plugin/events/LogMacros.hpp"

void BaseServer::setup_http_requests()
{
  this->_registry.get().add_system(
      [this](Registry&)
      {
        try {
          this->_http_client.handle_responses();
        } catch (HttpBadCode const& e) {
          this->_event_manager.get().emit<HttpBadCodeEvent>(e.code, e.what());
        }
      });

  SUBSCRIBE_EVENT(HttpBadCodeEvent, {
    LOGGER("http",
           LogLevel::ERROR,
           std::format("error {}: {}", event.code, event.message));
  });

  SUBSCRIBE_EVENT(ExposeServer, { this->register_server(event.host); })
}

void handle_register_response(void* raw_context, httplib::Result const& result)
{
  auto* context = static_cast<BaseServer*>(raw_context);

  auto parsed = parseJsonObject()(result->body);

  if (parsed.index() == ERR) {
    CONTEXT_LOGGER(
        context,
        "http",
        LogLevel::ERROR,
        "failed to parse http response: " + std::get<ERR>(parsed).message);
    return;
  }
  try {
    context->_server_id =
        std::get<int>(std::get<SUCCESS>(parsed).value.at("id").value);
  } catch (std::bad_variant_access const&) {
    CONTEXT_LOGGER(
        context, "http", LogLevel::WARNING, "wrong json type in resonse");
    return;
  } catch (std::out_of_range const&) {
    CONTEXT_LOGGER(context,
                   "http",
                   LogLevel::WARNING,
                   "wrong json type in resonse object");
    return;
  }
  CONTEXT_LOGGER(context,
                 "http",
                 LogLevel::INFO,
                 std::format("successfully registered server, server id: {}",
                             context->_server_id));
}

void BaseServer::register_server(std::string const& host)
{
  if (this->_port == -1) {
    LOGGER("http", LogLevel::WARNING, "server not launched yet, skipping")
  }
  this->_http_client.register_post(
      &handle_register_response,
      this,
      "/active_server",
      std::format(R"({}"ip":"{}","port":{},"game_name":"{}"{})",
                  '{',
                  host,
                  this->_port,
                  this->game_name,
                  '}'));
}

void BaseServer::unregister_server()
{
  if (this->_server_id == -1) {
    return;
  }
  this->_http_client.register_delete(
      nullptr,
      nullptr,
      "/active_server",
      std::format(R"({}"id":{}{})", '{', this->_server_id, '}'));
}
