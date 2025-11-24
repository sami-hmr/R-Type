#pragma once

#include <cerrno>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <variant>

#include "Json/JsonParser.hpp"
#include "ParserTypes.hpp"
#include "ecs/Registery.hpp"
#include "plugin/libLoaders/LDLoader.hpp"

class EntityLoader
{
public:
  explicit EntityLoader(Registery& registery)
      : _registery(registery)
  {
  }

  void load(std::string const& directory)
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

  void load_file(std::string const& filepath)
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
        JsonArray const& array = std::get<JsonArray>(r.at("entities").value);
        for (auto const& it : array) {
          this->load_entity(std::get<JsonObject>(it.value));
        }
      } catch (std::out_of_range&) {
        std::cerr << "Parsing \"" << filepath
                  << R"(": missing "entities" field)" << '\n';
      } catch (std::bad_variant_access&) {
        std::cerr << "Parsing \"" << filepath
                  << R"(": invalid "entities" value)" << '\n';
      }
    }
  }

  void load_entity(JsonObject const& config)
  {
    Registery::Entity new_entity = this->_registery.get().spawn_entity();
    for (auto const& [key, sub_config] : config) {
      std::string plugin = key.substr(0, key.find(':'));
      std::string comp = key.substr(key.find(':') + 1);
      try {
        if (!this->_loaders.contains(plugin)) {
          this->get_loader(plugin);
        }
      } catch (NotExistingLib const& e) {
        std::cerr << e.what() << '\n';
      }
      try {
        if (!this->_plugins.contains(plugin)) {
          this->_plugins.emplace(
              plugin,
              this->_loaders.at(plugin)->get_instance(
                  "entry_point", this->_registery.get(), *this));
        }
      } catch (LoaderException const& e) {
        std::cerr << e.what() << '\n';
      }
      this->_plugins.at(plugin)->set_component(
          new_entity, comp, sub_config.value);
    }
  }

private:
  void get_loader(std::string const& plugin)
  {
    this->_loaders.insert_or_assign(plugin,
                                    std::make_unique<
#if __linux__
                                        DlLoader
#endif
                                        <IPlugin>>("plugins/" + plugin));
  }

  std::unordered_map<std::string, std::unique_ptr<LibLoader<IPlugin>>>
      _loaders {};
  std::unordered_map<std::string, std::unique_ptr<IPlugin>> _plugins {};
  std::reference_wrapper<Registery> _registery;
};
