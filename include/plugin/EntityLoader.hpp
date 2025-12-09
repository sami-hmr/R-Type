#pragma once

#include <memory>
#include <string>
#include <typeindex>
#include <unordered_map>

#include "CustomException.hpp"
#include "Json/JsonParser.hpp"
#include "TwoWayMap.hpp"
#include "ecs/Registery.hpp"
#include "plugin/IPlugin.hpp"
#include "plugin/libLoaders/ILibLoader.hpp"

class EntityLoader
{
public:
  explicit EntityLoader(Registery& registery);

  void load(std::string const& directory);

  void load_file(std::string const& filepath);

  std::optional<Registery::Entity> load_entity(JsonObject const& config);
  void load_components(Registery::Entity e, JsonObject const& config);

  void load_plugin(std::string const& plugin,
                   std::optional<JsonObject> const& config = std::nullopt);

private:
  void load_scene(JsonObject& json_scene);
  void get_loader(std::string const& plugin);

  std::unordered_map<std::string, std::unique_ptr<LibLoader<IPlugin>>> _loaders;
  std::unordered_map<std::string, std::unique_ptr<IPlugin>> _plugins;
  std::reference_wrapper<Registery> _registery;
};

CUSTOM_EXCEPTION(BadComponentDefinition)
CUSTOM_EXCEPTION(UndefinedComponentValue)
