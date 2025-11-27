#pragma once

#include <memory>
#include <string>

#include "CustomException.hpp"
#include "Json/JsonParser.hpp"
#include "ecs/Registery.hpp"
#include "plugin/IPlugin.hpp"
#include "plugin/libLoaders/ILibLoader.hpp"

/**
 * @brief Loads entities and plugins from configuration files.
 */
class EntityLoader
{
public:
  explicit EntityLoader(Registery& registery);

  /**
   * @brief Recursively loads JSON files from a directory.
   */
  void load(std::string const& directory);

  /**
   * @brief Loads entities from a single JSON file.
   */
  void load_file(std::string const& filepath);

  /**
   * @brief Creates an entity from configuration data.
   */
  void load_entity(JsonObject const& config);

  /**
   * @brief Loads and initializes a plugin by name.
   */
  void load_plugin(std::string const& plugin);

private:
  void get_loader(std::string const& plugin);

  std::unordered_map<std::string, std::unique_ptr<LibLoader<IPlugin>>> _loaders;
  std::unordered_map<std::string, std::unique_ptr<IPlugin>> _plugins;
  std::reference_wrapper<Registery> _registery;
};

CUSTOM_EXCEPTION(BadComponentDefinition)
CUSTOM_EXCEPTION(UndefinedComponentValue)
