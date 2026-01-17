#include <format>
#include <unordered_set>
#include <vector>

#include "Controller.hpp"
#include "Json/JsonParser.hpp"
#include "plugin/components/Controllable.hpp"
#include "plugin/events/IoEvents.hpp"
#include "plugin/events/RebindingEvent.hpp"
#include "plugin/events/SceneChangeEvent.hpp"

static std::string get_current_scene_name(std::size_t id)
{
  return std::format("__bindings_scene__{}", id);
}

void Controller::create_binding_scene(std::size_t entity)
{
  auto const& event_map =
      this->_registry.get().get_components<Controllable>()[entity]->event_map;
  std::vector<std::pair<std::uint16_t, Controllable::Trigger>> bindings(
      event_map.begin(), event_map.end());
  std::sort(bindings.begin(),
            bindings.end(),
            [](auto const& f, auto const& s)
            { return f.second.first.second > s.second.first.second; });
  std::vector<RebindingScene> scenes;
  std::unordered_set<std::string> already_treated;

  std::size_t base_index = 0;
  for (auto const& [key, it] : bindings) {
    if (already_treated.contains(it.first.second)) {
      continue;
    }
    already_treated.emplace(it.first.second);
    if (base_index % 4 == 0) {
      scenes.emplace_back();
    }
    Key true_key = Key(key >> 8);  // NOLINT
    std::string const& key_str = KEY_MAPPING.at_second(true_key);
    scenes.rbegin()->elements.emplace_back(
        key_str, true_key << 8, it.first.second);  // NOLINT
    base_index++;
  }

  std::string current_scene;

  for (std::size_t index = 0; index < scenes.size(); index++) {
    current_scene = get_current_scene_name(index);
    _rebinding_scenes[current_scene].emplace_back(
        *this->_loader.get().load_entity_template(
            this->_current_binding_scene->background_template,
            {{this->_registry.get().get_component_key<Scene>(),
              Scene(current_scene).to_bytes()}}));  // BACKGROUND

    _rebinding_scenes[current_scene].emplace_back(
        *this->_loader.get().load_entity_template(
            this->_current_binding_scene->back_to_base_scene_template,
            {{this->_registry.get().get_component_key<Scene>(),
              Scene(current_scene).to_bytes()}},
            JsonObject({
                {"to_emit",
                 JsonValue(JsonArray({JsonValue(
                     JsonObject({{"ExitRebind", JsonValue(JsonObject())}}))}))},
            })));  // GO BACK

    _rebinding_scenes[current_scene].emplace_back(
        *this->_loader.get().load_entity_template(
            this->_current_binding_scene->text_template,
            {{this->_registry.get().get_component_key<Scene>(),
              Scene(current_scene).to_bytes()}},
            JsonObject({{"y", JsonValue(0.9)},  // NOLINT
                        {"height", JsonValue(0.05)},  // NOLINT
                        {"width", JsonValue(0.1)},  // NOLINT
                        {"z", JsonValue(1001)},  // NOLINT
                        {"text",
                         JsonValue(std::format(
                             "{}/{}",
                             index + 1,
                             scenes.size()))}})));  // SCENE ID INDICATOR
    if (index != 0) {
      _rebinding_scenes[current_scene].emplace_back(
          *this->_loader.get().load_entity_template(
              this->_current_binding_scene->link_template,
              {{this->_registry.get().get_component_key<Scene>(),
                Scene(current_scene).to_bytes()}},
              JsonObject(
                  {{"y", JsonValue(0.9)},  // NOLINT
                   {"x", JsonValue(-0.2)},  // NOLINT
                   {"height", JsonValue(0.05)},  // NOLINT
                   {"width", JsonValue(0.1)},  // NOLINT
                   {"z", JsonValue(1001)},  // NOLINT
                   {"text", JsonValue("<")},
                   {"to_emit",
                    JsonValue(JsonArray({JsonValue(JsonObject(
                        {{"SceneChangeEvent",
                          JsonValue(JsonObject(
                              {{"target_scene",
                                JsonValue(get_current_scene_name(index - 1))},
                               {"reason", JsonValue("")},
                               {"force", JsonValue(false)},  // NOLINT
                               {"main", JsonValue(true)}}))},
                         {"DisableSceneEvent",
                          JsonValue(JsonObject(
                              {{"target_scene",
                                JsonValue(
                                    current_scene)}}))}}))}))}})));  // SWITCH
                                                                     // SCENES
    }
    if (index < (scenes.size() - 1)) {
      _rebinding_scenes[current_scene].emplace_back(
          *this->_loader.get().load_entity_template(
              this->_current_binding_scene->link_template,
              {{this->_registry.get().get_component_key<Scene>(),
                Scene(current_scene).to_bytes()}},
              JsonObject(
                  {{"y", JsonValue(0.9)},  // NOLINT
                   {"x", JsonValue(0.2)},  // NOLINT
                   {"height", JsonValue(0.05)},  // NOLINT
                   {"width", JsonValue(0.1)},  // NOLINT
                   {"z", JsonValue(1001)},  // NOLINT
                   {"text", JsonValue(">")},
                   {"to_emit",
                    JsonValue(JsonArray({JsonValue(JsonObject(
                        {{"SceneChangeEvent",
                          JsonValue(JsonObject(
                              {{"target_scene",
                                JsonValue(get_current_scene_name(index + 1))},
                               {"reason", JsonValue("")},
                               {"force", JsonValue(false)},  // NOLINT
                               {"main", JsonValue(true)}}))},
                         {"DisableSceneEvent",
                          JsonValue(JsonObject(
                              {{"target_scene",
                                JsonValue(
                                    current_scene)}}))}}))}))}})));  // SWITCH
                                                                     // SCENES
    }
    std::size_t rebinder_index = 0;
    for (auto const& [key_str, true_key, desc] : scenes[index].elements) {
      _rebinding_scenes[current_scene].emplace_back(
          *this->_loader.get().load_entity_template(
              this->_current_binding_scene->button_template,
              {{this->_registry.get().get_component_key<Scene>(),
                Scene(current_scene).to_bytes()}},
              JsonObject({
                  {"y",
                   JsonValue((-0.3 + (rebinder_index * 0.325)))},  // NOLINT
                  {"z", JsonValue(1001)},  // NOLINT
                  {"text", JsonValue(std::format("{}: {}", key_str, desc))},
                  {"on_click",
                   JsonValue(JsonArray({JsonValue(JsonObject(
                       {{"WatchRebind",
                         JsonValue(JsonObject(
                             {{"entity", JsonValue(static_cast<int>(entity))},
                              {"key",
                               JsonValue(static_cast<std::uint16_t>(
                                   true_key))}}))}}))}))},
              })));
      rebinder_index++;
    }
  }
}

void Controller::delete_binding_scene(bool disable)
{
  for (auto const& [scene, entities] : this->_rebinding_scenes) {
    for (auto const& e : entities) {
      if (!disable && scene == "__rebinding_card__") {
        continue;
      }
      if (scene == "__rebinding_card__") {std::cout << "sjkfsdkjfhdskfds\n";};
      this->_registry.get().kill_entity(e);
    }
    if (disable) {
      this->_event_manager.get().emit<DisableSceneEvent>(scene);
    }
  }
  this->_rebinding_scenes.clear();
}
