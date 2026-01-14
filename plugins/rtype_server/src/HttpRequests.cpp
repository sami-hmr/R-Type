#include "BaseTypes.hpp"
#include "ByteParser/ByteParser.hpp"
#include "ParserTypes.hpp"
#include "RtypeServer.hpp"
#include "ecs/InitComponent.hpp"
#include "network/Httplib.hpp"
#include "network/server/Server.hpp"
#include "plugin/Byte.hpp"
#include "plugin/events/LogMacros.hpp"
#include "plugin/events/NetworkEvents.hpp"
#include "plugin/events/SceneChangeEvent.hpp"

void handle_get_player_save(void* raw_context, httplib::Result const& result)
{
  // std::cout << "user_save: " << result->body << std::endl;
  auto* context = static_cast<RtypeServer*>(raw_context);
  auto const& id_it = result->headers.find("id");
  if (id_it == result->headers.end()) {
    CONTEXT_LOGGER(
        context, "http", LogLevel::ERROR, "no id in header, skipping");
    return;
  }
  auto parsed = parseInt()(id_it->second);

  if (parsed.index() == ERR) {
    CONTEXT_LOGGER(context,
                   "http",
                   LogLevel::ERROR,
                   "error parsing id in header, skipping");
    return;
  }
  int user = std::get<SUCCESS>(parsed).value;
  std::size_t client = context->get_client_by_user(user);

  if (result->body == "NOT FOUND") {
    context->_loader.get().load_components(
        context->_users_entities[user],
        JsonObject({{"template", JsonValue("player")}}));
    init_component<Scene>(context->_registry.get(),
                          context->_event_manager.get(),
                          context->_users_entities[user],
                          "game");
  } else {
    auto parsed = parseByteArray(
        parseBytePair(parseByteString(), parseByteArray(parseByte<Byte>())))(
        ByteArray(result->body.begin(), result->body.end()));

    if (parsed.index() == ERR) {
      CONTEXT_LOGGER(
          context, "http", LogLevel::ERROR, "error parsing save in body");
      context->_loader.get().load_components(
          context->_users_entities[user],
          JsonObject({{"template", JsonValue("player")}}));
      init_component<Scene>(context->_registry.get(),
                            context->_event_manager.get(),
                            context->_users_entities[user],
                            "game");
    } else {
      for (auto const& comp : std::get<SUCCESS>(parsed).value) {
        init_component(context->_registry.get(),
                       context->_event_manager.get(),
                       context->_users_entities[user],
                       comp.first,
                       comp.second);
      }
    }
  }

  context->_event_manager.get().emit<StateTransfer>(client);

  context->_event_manager.get().emit<EventBuilderId>(
      client,
      "SceneChangeEvent",
      SceneChangeEvent("loby", "", /*f=*/true).to_bytes());
}

void RtypeServer::ask_player_save(int user_id)
{
  this->_http_client.register_post(
      &handle_get_player_save,
      this,
      "/get_save",
      std::format(
          R"({}"id":{},"game":"{}"{})", '{', user_id, this->game_name, '}'));
}

void handle_save_response(void* /**/, httplib::Result const& /**/) {}

void RtypeServer::save_player(int user_id)
{
  try {
    std::size_t client = this->get_client_by_user(user_id);

    for (auto const& it : this->_player_entities) {
      if (it.second != client) {
        continue;
      }
      std::size_t entity = it.first;
      ByteArray save = this->_registry.get().get_byte_entity(entity);
      this->_http_client.register_post(&handle_save_response,
                                       this,
                                       "/save",
                                       std::string(save.begin(), save.end()),
                                       "application/octet-stream",
                                       {{"user-id", std::format("{}", user_id)},
                                        {"game-name", this->game_name}});
    }

  } catch (ClientNotFound const&) {
  }
}
