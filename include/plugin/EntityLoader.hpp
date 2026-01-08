#pragma once

#include <functional>
#include <memory>
#include <string>
#include <typeindex>
#include <unordered_map>

#include "CustomException.hpp"
#include "Json/JsonParser.hpp"
#include "NetworkShared.hpp"
#include "TwoWayMap.hpp"
#include "ecs/EventManager.hpp"
#include "ecs/Registry.hpp"
#include "plugin/IPlugin.hpp"
#include "plugin/libLoaders/ILibLoader.hpp"

class EntityLoader
{
public:
  explicit EntityLoader(Registry& registry, EventManager& em);
  void load(std::string const& directory);

  void load_file(std::string const& filepath);

  std::optional<Registry::Entity> load_entity(JsonObject const& config);
  void load_components(Registry::Entity e, JsonObject const& config);

  void load_plugin(std::string const& plugin,
                   std::optional<JsonObject> const& config = std::nullopt);

  void load_byte_component(
      Registry::Entity entity,
      ComponentBuilder const& component,
      TwoWayMap<Registry::Entity, Registry::Entity> const& indexes);

  bool is_plugin_loaded(std::string const& plugin);

private:
  void load_scene(JsonObject& json_scene);
  void get_loader(std::string const& plugin);

  std::unordered_map<std::string, std::unique_ptr<LibLoader<IPlugin>>> _loaders;
  std::unordered_map<std::string, std::unique_ptr<IPlugin>> _plugins;
  std::reference_wrapper<Registry> _registry;
  std::reference_wrapper<EventManager> _event_manager;
};

CUSTOM_EXCEPTION(BadComponentDefinition)
CUSTOM_EXCEPTION(UndefinedComponentValue)
