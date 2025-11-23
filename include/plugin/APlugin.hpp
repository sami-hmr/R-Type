#pragma once

#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include "EntityLoader.hpp"
#include "Json/JsonParser.hpp"
#include "ecs/Registery.hpp"
#include "plugin/IPlugin.hpp"

class APlugin : public IPlugin
{
public:
  APlugin(Registery& registery,
          EntityLoader& loader,
          std::unordered_map<
              std::string,
              std::function<void(Registery::Entity, JsonVariant const&)>>
              components)
      : components_(components)
      , registery_(registery)
      , loader_(loader)
  {
  }

  void setComponent(Registery::Entity entity,
                    std::string const& key,
                    JsonVariant const& config) override
  {
    try {
      this->components_.at(key)(entity, config);
    } catch (std::out_of_range const&) {
      std::cerr << key << ": unknow component" << std::endl;
    }
  }

protected:
  const std::unordered_map<
      std::string,
      std::function<void(Registery::Entity, JsonVariant const&)>>
      components_;
  std::vector<std::string> depends_on_;
  std::reference_wrapper<Registery> registery_;
  std::reference_wrapper<EntityLoader> loader_;
};

#define COMP_INIT(comp_name, method_name) \
  {#comp_name, \
   [this](size_t entity, JsonVariant config) \
   { this->method_name(entity, config); }}
