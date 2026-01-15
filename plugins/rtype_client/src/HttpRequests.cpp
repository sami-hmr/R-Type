#include <cstddef>
#include <format>
#include <string>
#include <utility>
#include <vector>

#include "../plugins/rtype_client/include/RtypeClient.hpp"
#include "Json/JsonParser.hpp"
#include "ecs/Scenes.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/components/Clickable.hpp"
#include "plugin/components/Position.hpp"
#include "plugin/components/Text.hpp"
#include "plugin/events/HttpEvents.hpp"
#include "plugin/events/SceneChangeEvent.hpp"

void RtypeClient::handle_http()
{
  SUBSCRIBE_EVENT(HttpBadCodeEvent, {
    this->alert(std::format("error {}: {}", event.code, event.message));
  });

  SUBSCRIBE_EVENT(Save, {
    this->_event_manager.get().emit<EventBuilder>(
        "SavePlayer", SavePlayer(this->_user_id).to_bytes());
  })

  SUBSCRIBE_EVENT(LoginSuccessfull, {
    this->_event_manager.get().emit<DisableSceneEvent>("login");
    this->_event_manager.get().emit<DisableSceneEvent>("register");

    this->_event_manager.get().emit<SceneChangeEvent>(
        "server_search", "connected", false);
    this->_event_manager.get().emit<FetchAvailableServers>();
  })

  SUBSCRIBE_EVENT(FetchAvailableServersSuccessfull, {
    for (auto const& it : this->_server_fetch_entities) {
      this->_registry.get().kill_entity(it);
    }
    this->_server_fetch_entities.clear();
    this->_event_manager.get().emit<DisableSceneEvent>(
        this->_current_server_fetch_scene);
    this->handle_server_fetched();
  })
}

static std::string get_current_scene_name(std::size_t id)
{
  return std::format("server_search_{}", id);
}

void RtypeClient::handle_server_fetched()
{
  std::size_t scene_id = 0;
  std::vector<std::pair<std::string, std::vector<AvailableServer>>> scenes = {
      {get_current_scene_name(scene_id), {}}};

  for (auto const& it : this->_available_servers) {
    if (scenes.rbegin()->second.size() >= 4) {
      scene_id += 1;
      scenes.emplace_back(get_current_scene_name(scene_id),
                          std::vector<AvailableServer> {});
    }
    scenes.rbegin()->second.emplace_back(it);
  }

  for (std::size_t i = 0; i < scenes.size(); i++) {
    auto const& it = scenes[i];
    this->_registry.get().add_scene(it.first);

    this->_server_fetch_entities.emplace_back(
        *this->_loader.get().load_entity_template(
            "page_index_indicator",
            {
                {this->_registry.get().get_component_key<Scene>(),
                 Scene(it.first).to_bytes()},
            },
            {{"text", std::format("{}/{}", i + 1, scenes.size())}}));

    if (i != 0) {
      this->_server_fetch_entities.emplace_back(
          *this->_loader.get().load_entity_template(
              "switch_pages_search_button",
              {{this->_registry.get().get_component_key<Scene>(),
                Scene(it.first).to_bytes()}},
              {{"x", -0.3},
               {"text", "<"},
               {"target_scene", scenes[i - 1].first},
               {"current_scene", it.first}}));
    }
    if (i < (scenes.size() - 1)) {
      this->_server_fetch_entities.emplace_back(
          *this->_loader.get().load_entity_template(
              "switch_pages_search_button",
              {{this->_registry.get().get_component_key<Scene>(),
                Scene(it.first).to_bytes()}},
              {{"x", 0.3},
               {"text", ">"},
               {"target_scene", scenes[i + 1].first},
               {"current_scene", it.first}}));
    }
    for (std::size_t index = 0; index < it.second.size(); index++) {
      auto const& server = it.second[index];
      std::size_t card_entity = *this->_loader.get().load_entity_template(
          "server_card",
          {{this->_registry.get().get_component_key<Scene>(),
            Scene(it.first).to_bytes()},
           {this->_registry.get().get_component_key<Position>(),
            Position(0.0, -0.3 + (index * 0.325)).to_bytes()},
           {this->_registry.get().get_component_key<Clickable>(),
            Clickable(std::vector<std::pair<std::string, JsonObject>>(
                          {{"ClientConnection",
                            {{"host", server.address},
                             {"port", static_cast<int>(server.port)}}},
                           {"SceneChangeEvent",
                            {{"target_scene", "connecting_card"},
                             {"reason", "click"},
                             {"force", false}}}}))
                .to_bytes()}});
      if (this->_registry.get().has_component<Text>(card_entity)) {
        this->_registry.get().get_components<Text>()[card_entity]->text =
            std::format("id: {}, host: {}, port: {}",
                        server.id,
                        server.address,
                        server.port);
      }
      this->_server_fetch_entities.emplace_back(card_entity);
    }
  }
  this->_event_manager.get().emit<SceneChangeEvent>(
      scenes.begin()->first, "", false);
}
