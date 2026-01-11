#include <variant>

#include "Json/JsonParser.hpp"
#include "ParserTypes.hpp"
#include "ServerLaunch.hpp"
#include "network/Httplib.hpp"
#include "network/server/BaseServer.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/events/LogMacros.hpp"
#include "plugin/events/HttpEvents.hpp"

void BaseServer::setup_http_requests()
{
  this->_registry.get().add_system(
      [this](Registry&) { this->_http_client.handle_responses(this); });

  SUBSCRIBE_EVENT(ExposeServer, {this->register_server(event.host);})
}

void handle_register_response(void* raw_context, httplib::Result const& result)
{
  auto* context = static_cast<BaseServer*>(raw_context);

  auto parsed = parseJsonObject()(result->body);

  if (parsed.index() == ERROR) {
    CONTEXT_LOGGER(
        context,
        "http",
        LogLevel::ERROR,
        "failed to parse http response: " + std::get<ERROR>(parsed).message);
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
      "/active_server",
      std::format(R"({}"ip":"{}","port":{},"game_name":"{}"{})", '{', host, this->_port, this->game_name, '}'));
}
