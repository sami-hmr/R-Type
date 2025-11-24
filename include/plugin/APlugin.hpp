#pragma once

#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include "IPlugin.hpp"
#include "Json/JsonParser.hpp"
#include "ecs/Registery.hpp"
#include "EntityLoader.hpp"
#include "plugin/IPlugin.hpp"

class APlugin : public IPlugin
{
public:
  APlugin(Registery& registery,
          EntityLoader& loader,
          std::unordered_map<
              std::string,
              std::function<void(Registery::entity, JsonVariant const&)>>
              components)
      : components(components)
      , _registery(registery)
      , _loader(loader)
  {
  }

  void set_component(Registery::entity entity,
                    std::string const& key,
                    JsonVariant const& config) override
  {
    try {
      this->components.at(key)(entity, config);
    } catch (std::out_of_range const&) {
      std::cerr << key << ": unknow component" << '\n';
    }
  }

protected:
  const std::unordered_map<
      std::string,
      std::function<void(Registery::entity, JsonVariant const&)>>
      components{};
  std::vector<std::string> _depends_on;
  std::reference_wrapper<Registery> _registery;
  std::reference_wrapper<EntityLoader> _loader;
};

#define COMP_INIT(comp_name, method_name) \
  { \
    #comp_name, [this](size_t entity, JsonVariant config) \
    { this->method_name(entity, config); } \
  }
