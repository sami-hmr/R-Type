#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

#include "plugin/EntityLoader.hpp"

#include <sys/types.h>

#include "Json/JsonParser.hpp"
#include "ecs/EventManager.hpp"
#include "ecs/InitComponent.hpp"
#include "ecs/Registry.hpp"
#include "ecs/Scenes.hpp"
#include "plugin/libLoaders/LDLoader.hpp"
#ifdef _WIN32
#  include "plugin/libLoaders/WindowsLoader.hpp"
#endif

EntityLoader::EntityLoader(Registry& registry, EventManager& em)
    : _registry(std::ref(registry))
    , _event_manager(std::ref(em))
{
}

bool EntityLoader::is_plugin_loaded(std::string const& plugin)
{
  return _plugins.contains(plugin);
}

void EntityLoader::load(std::string const& directory)
{
  try {
    std::vector<std::filesystem::path> entries;

    for (const auto& entry : std::filesystem::directory_iterator(directory)) {
      entries.push_back(entry.path());
    }

    std::sort(entries.begin(), entries.end());

    for (const auto& path : entries) {
      if (std::filesystem::is_regular_file(path) && path.extension() == ".json")
      {
        this->load_file(path.string());
      } else if (std::filesystem::is_directory(path)) {
        this->load(path.string());
      }
    }
  } catch (std::filesystem::filesystem_error const& e) {
    std::cerr << "failed to read directory: " << e.what() << '\n';
  }
}

void EntityLoader::load_scene(JsonObject& json_scene)
{
  std::string scene = "default";
  SceneState scene_state = SceneState::DISABLED;

  if (json_scene.contains("name")) {
    scene = std::get<std::string>(json_scene.at("name").value);
  }

  if (json_scene.contains("state")) {
    std::string scene_state_tmp =
        std::get<std::string>(json_scene.at("state").value);
    try {
      scene_state = SCENE_STATE_STR.at_second(scene_state_tmp);
    } catch (std::out_of_range&) {
      std::cerr << "Scene: " << scene_state_tmp << " not found\n";
    }
  }

  this->_registry.get().add_scene(scene, scene_state);

  if (json_scene.contains("plugins")) {
    JsonArray const& plugin_array =
        std::get<JsonArray>(json_scene.at("plugins").value);
    for (auto const& it : plugin_array) {
      JsonObject config_obj = std::get<JsonObject>(it.value);
      if (config_obj.contains("name")) {
        std::string name = std::get<std::string>(config_obj.at("name").value);
        if (config_obj.contains("config")) {
          JsonObject config =
              std::get<JsonObject>(config_obj.at("config").value);
          this->load_plugin(name, config);
        } else {
          this->load_plugin(name);
        }
      }
    }
  }

  if (json_scene.contains("entities")) {
    JsonArray const& array =
        std::get<JsonArray>(json_scene.at("entities").value);
    for (auto const& it : array) {
      std::optional<Registry::Entity> new_e =
          this->load_entity(std::get<JsonObject>(it.value));
      if (new_e.has_value()) {
        this->_registry.get().add_component(new_e.value(), Scene(scene));
      }
    }
  }
}

void EntityLoader::load_file(std::string const& filepath)
{
  std::ifstream infi(filepath);
  if (infi.fail()) {
    std::cerr << "failed to open file \"" << filepath << "\": " << errno
              << '\n';
    return;
  }
  std::string str((std::istreambuf_iterator<char>(infi)),
                  std::istreambuf_iterator<char>());

  Result<JsonObject> result = parseJsonObject()(Rest(str));

  if (result.index() == ERR) {
    printError(std::get<ERR>(result), str, filepath);
  } else {
    JsonObject r = std::get<SUCCESS>(result).value;
    try {
      bool old_format = true;
      if (r.contains("entities_template")) {
        JsonArray const& templates_array =
            std::get<JsonArray>(r.at("entities_template").value);
        for (auto const& template_it : templates_array) {
          JsonObject template_obj = std::get<JsonObject>(template_it.value);
          JsonObject default_parameters;
          if (template_obj.contains("default_parameters")) {
            default_parameters = std::get<JsonObject>(
                template_obj.at("default_parameters").value);
          }
          this->_registry.get().add_template(
              std::get<std::string>(template_obj.at("name").value),
              std::get<JsonObject>(template_obj.at("components").value),
              default_parameters);
        }
        old_format = false;
      }
      if (r.contains("scenes")) {
        JsonArray const& scenes_array =
            std::get<JsonArray>(r.at("scenes").value);
        for (auto const& scene_it : scenes_array) {
          JsonObject scene_obj = std::get<JsonObject>(scene_it.value);
          this->load_scene(scene_obj);
        }
        old_format = false;
      }
      if (r.contains("configs")) {
        JsonArray const& configs_array =
            std::get<JsonArray>(r.at("configs").value);
        for (auto const& configs_it : configs_array) {
          this->load(std::get<std::string>(configs_it.value));
        }
        old_format = false;
      }
      if (old_format) {
        this->load_scene(r);
      }
    } catch (std::out_of_range&) {
      std::cerr << "Parsing \"" << filepath << R"(": missing field)" << '\n';
    } catch (std::bad_variant_access&) {
      std::cerr << "Parsing \"" << filepath << R"(": invalid value)" << '\n';
    }
  }
}

void EntityLoader::load_plugin(std::string const& plugin,
                               std::optional<JsonObject> const& config)
{
  this->get_loader(plugin);
  if (!this->_plugins.contains(plugin)) {
    try {
      this->_plugins[plugin];
      this->_plugins.insert_or_assign(
          plugin,
          this->_loaders.at(plugin)->get_instance("entry_point",
                                                  this->_registry.get(),
                                                  this->_event_manager.get(),
                                                  *this,
                                                  config));
    } catch (LoaderException const& e) {
      std::cerr << e.what() << '\n';
    }
  }
}

void EntityLoader::load_components(Registry::Entity e, JsonObject const& config)
{
  if (config.contains("template")) {
    std::string name = std::get<std::string>(config.at("template").value);
    JsonObject params = {};
    if (config.contains("parameters")) {
      params = std::get<JsonObject>(config.at("parameters").value);
    }
    this->load_components(e, this->_registry.get().get_template(name, params));
    if (config.contains("config")) {
      this->load_components(e, std::get<JsonObject>(config.at("config").value));
    }
    return;
  }

  for (auto const& [key, sub_config] : config) {
    std::string plugin = key.substr(0, key.find(':'));
    std::string comp = key.substr(key.find(':') + 1);

    try {
      this->load_plugin(plugin);
      this->_plugins.at(plugin)->set_component(e, comp, sub_config.value);
    } catch (BadComponentDefinition const& e) {
      std::cerr
          << std::format(
                 "Error creating component {} in plugin {}: Bad component "
                 "definition: " "{}\n",
                 comp,
                 plugin,
                 e.what());
      return;
    } catch (UndefinedComponentValue const& e) {
      std::cerr << std::format(
          "Error creating component {} in plugin {}: undefined value: {}\n",
          comp,
          plugin,
          e.what());
      return;
    }
  }
}

std::optional<Registry::Entity> EntityLoader::load_entity(
    JsonObject const& config)
{
  Registry::Entity new_entity = this->_registry.get().spawn_entity();
  this->load_components(new_entity, config);
  return new_entity;
}

std::optional<Registry::Entity> EntityLoader::load_entity_template(
    std::string const& template_name,
    std::vector<std::pair<std::string, ByteArray>> const& aditionals,
    JsonObject const& parameters)
{
  auto const& entity =
      this->load_entity(JsonObject({{"template", JsonValue(template_name)},
                                    {"parameters", JsonValue(parameters)}}));

  if (!entity) {
    // LOGGER("load entity template",
    //        LogLevel::ERROR,
    //        "failed to load entity template " + event.template_name);
    return std::nullopt;
  }
  for (auto const& [id, comp] : aditionals) {
    init_component(
        this->_registry.get(), this->_event_manager.get(), *entity, id, comp);
  }
  return entity;
}

void EntityLoader::get_loader(std::string const& plugin)
{
  try {
    if (!this->_loaders.contains(plugin)) {
      this->_loaders[plugin];
      this->_loaders.insert_or_assign(
          plugin,
          std::make_unique<
#ifdef _WIN32
              WindowsLoader
#elif __linux__
              DlLoader
#endif
              <IPlugin>>("build/plugins/" + plugin));
    }
  } catch (NotExistingLib const& e) {
    std::cerr << e.what() << '\n';
  }
}

void EntityLoader::load_byte_component(
    Registry::Entity entity,
    ComponentBuilder const& component,
    TwoWayMap<Registry::Entity, Registry::Entity> const& indexes)
{
  if (component.id.contains(':')) {
    std::string plugin = component.id.substr(0, component.id.find(':'));
    this->load_plugin(plugin);
    if (this->_plugins.contains(plugin)) {
      init_component(this->_registry.get(),
                     this->_event_manager.get(),
                     entity,
                     component.id,
                     this->_registry.get().convert_comp_entity(
                         component.id, component.data, indexes.get_first()));
    }
  } else {
    init_component(this->_registry.get(),
                   this->_event_manager.get(),
                   entity,
                   component.id,
                   this->_registry.get().convert_comp_entity(
                       component.id, component.data, indexes.get_first()));
  }
}
