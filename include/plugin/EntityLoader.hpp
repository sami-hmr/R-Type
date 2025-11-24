#pragma once

#include <cerrno>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <variant>

#include "IPlugin.hpp"
#include "Json/JsonParser.hpp"
#include "ParserTypes.hpp"
#include "ecs/Registery.hpp"
#include "libLoaders/ILibLoader.hpp"
#include "plugin/libLoaders/LDLoader.hpp"

class EntityLoader
{
public:
  EntityLoader(Registery& registery)
      : registery_(registery)
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
      std::cerr << "failed to read directory: " << e.what() << std::endl;
    }
  }

  void load_file(std::string const& filepath)
  {
    std::ifstream infi(filepath);
    if (infi.fail()) {
      std::cerr << "failed to open file \"" << filepath
                << "\": " << std::strerror(errno) << std::endl;
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
          this->loadEntity(std::get<JsonObject>(it.value));
        }
      } catch (std::out_of_range&) {
        std::cerr << "Parsing \"" << filepath
                  << "\": missing \"entities\" field" << std::endl;
      } catch (std::bad_variant_access&) {
        std::cerr << "Parsing \"" << filepath
                  << "\": invalid \"entities\" value" << std::endl;
      }
    }
  }

  void loadEntity(JsonObject const& config)
  {
    Registery::Entity new_entity = this->registery_.get().spawn_entity();
    for (auto const& [key, sub_config] : config) {
      std::string plugin = key.substr(0, key.find(":"));
      std::string comp = key.substr(key.find(":") + 1);
      try {
        if (!this->loaders_.contains(plugin)) {
#if __linux__
          this->loaders_.emplace(
              plugin, std::make_unique<DlLoader<IPlugin>>("plugins/" + plugin));
#endif
        }
      } catch (NotExistingLib const& e) {
        std::cerr << e.what() << std::endl;
      }
      try {
        if (!this->plugins_.contains(plugin)) {
          this->plugins_.emplace(plugin,
                                 this->loaders_.at(plugin)->getInstance(
                                     "entry_point", this->registery_, *this));
        }
      } catch (LoaderException const& e) {
        std::cerr << e.what() << std::endl;
      }
      this->plugins_.at(plugin)->setComponent(
          new_entity, comp, sub_config.value);
    }
  }

public:  // private:
  std::unordered_map<std::string, std::unique_ptr<LibLoader<IPlugin>>> loaders_;
  std::unordered_map<std::string, std::unique_ptr<IPlugin>> plugins_;
  std::reference_wrapper<Registery> registery_;
};
