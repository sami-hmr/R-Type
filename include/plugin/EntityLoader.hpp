#pragma once

#include <memory>
#include <string>

#include "CustomException.hpp"
#include "Json/JsonParser.hpp"
#include "ecs/Registery.hpp"
#include "plugin/IPlugin.hpp"
#include "plugin/libLoaders/ILibLoader.hpp"

class EntityLoader
{
public:
  explicit EntityLoader(Registery& registery);

  void load(std::string const& directory);

  void load_file(std::string const& filepath);

  void load_entity(JsonObject const& config);

  void load_plugin(std::string const& plugin);

private:
  void get_loader(std::string const& plugin);

  std::unordered_map<std::string, std::unique_ptr<LibLoader<IPlugin>>> _loaders;
  std::unordered_map<std::string, std::unique_ptr<IPlugin>> _plugins;
  std::reference_wrapper<Registery> _registery;
};

CUSTOM_EXCEPTION(BadComponentDefinition)
CUSTOM_EXCEPTION(UndefinedComponentValue)
