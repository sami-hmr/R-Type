#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include "plugin/EntityLoader.hpp"

#include <sys/types.h>

#include "Json/JsonParser.hpp"
#include "ecs/Registery.hpp"
#include "plugin/libLoaders/LDLoader.hpp"

EntityLoader::EntityLoader(Registery& registery)
    : _registery(registery)
{
}

void EntityLoader::load(std::string const& directory)
{
  try {
    for (const auto& entry : std::filesystem::directory_iterator(directory)) {
      if (entry.is_regular_file() && entry.path().extension() == ".json") {
        this->load_file(entry.path().string());
      } else if (entry.is_directory()) {
        this->load(entry.path().string());
      }
    }
  } catch (std::filesystem::filesystem_error const& e) {
    std::cerr << "failed to read directory: " << e.what() << '\n';
  }
}

void EntityLoader::load_file(std::string const& filepath)
{
  std::ifstream infi(filepath);
  if (infi.fail()) {
    std::cerr << "failed to open file \"" << filepath << "\": " << errno
              << '\n';
  }
  std::string str((std::istreambuf_iterator<char>(infi)),
                  std::istreambuf_iterator<char>());

  Result<JsonObject> result = parseJsonObject()(Rest(str));

  if (result.index() == ERROR) {
    printError(std::get<ERROR>(result), str, filepath);
  } else {
    JsonObject r = std::get<SUCCESS>(result).value;
    try {
      JsonArray const& plugin_array =
          std::get<JsonArray>(r.at("plugins").value);
      for (auto const& it : plugin_array) {
        this->load_plugin(std::get<std::string>(it.value));
      }
      JsonArray const& array = std::get<JsonArray>(r.at("entities").value);
      for (auto const& it : array) {
        this->load_entity(std::get<JsonObject>(it.value));
      }
    } catch (std::out_of_range&) {
      std::cerr << "Parsing \"" << filepath << R"(": missing "entities" field)"
                << '\n';
    } catch (std::bad_variant_access&) {
      std::cerr << "Parsing \"" << filepath << R"(": invalid "entities" value)"
                << '\n';
    }
  }
}

void EntityLoader::load_plugin(std::string const& plugin)
{
  this->get_loader(plugin);
  if (!this->_plugins.contains(plugin)) {
    try {
      this->_plugins[plugin];
      this->_plugins.insert_or_assign(
          plugin,
          this->_loaders.at(plugin)->get_instance(
              "entry_point", this->_registery.get(), *this));
    } catch (LoaderException const& e) {
      std::cerr << e.what() << '\n';
    }
  }
}

void EntityLoader::load_entity(JsonObject const& config)
{
  Registery::Entity new_entity = this->_registery.get().spawn_entity();
  for (auto const& [key, sub_config] : config) {
    std::string plugin = key.substr(0, key.find(':'));
    std::string comp = key.substr(key.find(':') + 1);

    try {
      this->load_plugin(plugin);
      this->_plugins.at(plugin)->set_component(
          new_entity, comp, sub_config.value);
    } catch (BadComponentDefinition const& e) {
      std::cerr
          << std::format(
                 "Error creating component {} in plugin {}: Bad component "
                 "definition: " "{}\n",
                 comp,
                 plugin,
                 e.what());
    } catch (UndefinedComponentValue const& e) {
      std::cerr << std::format(
          "Error creating component {} in plugin {}: undefined value: {}\n",
          comp,
          plugin,
          e.what());
    }
  }
}

void EntityLoader::get_loader(std::string const& plugin)
{
  try {
    if (!this->_loaders.contains(plugin)) {
      this->_loaders[plugin];
      this->_loaders.insert_or_assign(plugin,
                                      std::make_unique<
#if __linux__
                                          DlLoader
#endif
                                          <IPlugin>>("plugins/" + plugin));
    }
  } catch (NotExistingLib const& e) {
    std::cerr << e.what() << '\n';
  }
}
