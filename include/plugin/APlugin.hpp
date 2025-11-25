#pragma once

#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>

#include "EntityLoader.hpp"
#include "IPlugin.hpp"
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
      : components(std::move(components))
      , _registery(registery)
      , _loader(loader)
  {
  }

  void set_component(Registery::Entity entity,
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
      std::function<void(Registery::Entity, JsonVariant const&)>>
      components;
  std::vector<std::string> _depends_on;
  std::reference_wrapper<Registery> _registery;
  std::reference_wrapper<EntityLoader> _loader;
};

#define COMP_INIT(comp_name, method_name) \
  {#comp_name, \
   [this](size_t entity, const JsonVariant& config) \
   { this->method_name(entity, config); }}
