#include <format>

#include "Controller.hpp"
#include "Json/JsonParser.hpp"
#include "plugin/events/IoEvents.hpp"
#include "plugin/events/SceneChangeEvent.hpp"

static std::string get_current_scene_name(std::string const& scene_name,
                                          std::size_t id)
{
  if (id == 0) {
    return scene_name;
  }
  return std::format("{}_{}", scene_name, id);
}

void Controller::create_binding_scene(std::size_t entity,
                                      std::string const& scene_name)
{
  auto const& bindings =
  this->_registry.get().get_components<Controllable>()[entity]->event_map;
  std::cout << "\"" << bindings.size() << "\"\n";
  std::string current_scene;
  std::size_t max_scene = 1 + ((bindings.size() - 1) / 4);

  std::size_t i = 0;
  for (auto const& [key, it] : bindings) {
    std::size_t index = i % 4;
    std::size_t scene_id = i / 4;
    if (index == 0) {
      current_scene = get_current_scene_name(scene_name, scene_id);
      _rebinding_scenes[current_scene].emplace_back(
          *this->_loader.get().load_entity_template(
              "card",
              {{this->_registry.get().get_component_key<Scene>(),
                Scene(current_scene).to_bytes()}},
              JsonObject({{"height", JsonValue(10.0)},  // NOLINT
                          {"width", JsonValue(10.0)},  // NOLINT
                          {"z", JsonValue(1000)}})));  // NOLINT

      _rebinding_scenes[current_scene].emplace_back(
          *this->_loader.get().load_entity_template(
              "button",
              {{this->_registry.get().get_component_key<Scene>(),
                Scene(current_scene).to_bytes()}},
              JsonObject({{"y", JsonValue(-0.6)},  // NOLINT
                          {"z", JsonValue(1001)},  // NOLINT
                          {"text", JsonValue("go back")},
                          {"on_click",
                           JsonValue(JsonArray({JsonValue(JsonObject(
                               {{"ExitRebind", JsonValue(JsonObject())}}))}))},
                          {}})));
      _rebinding_scenes[current_scene].emplace_back(
          *this->_loader.get().load_entity_template(
              "text",
              {{this->_registry.get().get_component_key<Scene>(),
                Scene(current_scene).to_bytes()}},
              JsonObject(
                  {{"y", JsonValue(0.9)},  // NOLINT
                   {"height", JsonValue(0.05)},  // NOLINT
                   {"width", JsonValue(0.1)},  // NOLINT
                   {"z", JsonValue(1001)},  // NOLINT
                   {"text",
                    JsonValue(std::format(
                        "{}/{}", scene_id + 1, max_scene))}})));  // NOLINT
      if (scene_id != 0) {
        _rebinding_scenes[current_scene].emplace_back(
            *this->_loader.get().load_entity_template(
                "link",
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
                                  JsonValue(get_current_scene_name(
                                      scene_name, scene_id - 1))},
                                 {"reason", JsonValue("")},
                                 {"force", JsonValue(false)},  // NOLINT
                                 {"main", JsonValue(true)}}))},
                           {"DisableSceneEvent",
                            JsonValue(JsonObject(
                                {{"target_scene",
                                  JsonValue(
                                      current_scene)}}))}}))}))}})));  // NOLINT
      }
      if (scene_id != (max_scene - 1)) {
        _rebinding_scenes[current_scene].emplace_back(
            *this->_loader.get().load_entity_template(
                "link",
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
                                  JsonValue(get_current_scene_name(
                                      scene_name, scene_id + 1))},
                                 {"reason", JsonValue("")},
                                 {"force", JsonValue(false)},  // NOLINT
                                 {"main", JsonValue(true)}}))},
                           {"DisableSceneEvent",
                            JsonValue(JsonObject(
                                {{"target_scene",
                                  JsonValue(
                                      current_scene)}}))}}))}))}})));  // NOLINT
      }
    }
    Key true_key = Key(key >> 8);  // NOLINT
    std::string const& key_str = KEY_MAPPING.at_second(true_key);
    _rebinding_scenes[current_scene].emplace_back(
        *this->_loader.get().load_entity_template(
            "button",
            {{this->_registry.get().get_component_key<Scene>(),
              Scene(current_scene).to_bytes()}},
            JsonObject({
                {"y", JsonValue((-0.3 + (index * 0.325)))},  // NOLINT
                {"z", JsonValue(1001)},  // NOLINT
                {"text",
                 JsonValue(std::format("{}: {}", key_str, it.first.second))},
                {"on_click",
                 JsonValue(JsonArray({JsonValue(JsonObject(
                     {{"WatchRebind",
                       JsonValue(JsonObject(
                           {{"entity", JsonValue(static_cast<int>(entity))},
                            {"key",
                             JsonValue(static_cast<std::uint16_t>(
                                 true_key << 8))}}))}}))}))},
            })));

    i++;
  };
}

void Controller::delete_binding_scene(bool disable)
{
  for (auto const& [scene, entities] : this->_rebinding_scenes) {
    for (auto const& e : entities) {
      this->_registry.get().kill_entity(e);
    }
    if (disable) {
      this->_event_manager.get().emit<DisableSceneEvent>(scene);
    }
  }
  this->_rebinding_scenes.clear();
}
