#include <iostream>
#include <vector>
#include "plugin/APlugin.hpp"

APlugin::APlugin(Registery& registery,
        EntityLoader& loader,
        std::vector<std::string> const &depends_on,
        std::unordered_map<
            std::string,
            std::function<void(Registery::Entity, JsonVariant const&)>>
            components)
    : components(std::move(components))
    , _registery(registery)
    , _loader(loader)
{
    for (auto const &p : depends_on) {
        this->_loader.get().load_plugin(p);
    }
}

void APlugin::set_component(Registery::Entity entity,
                   std::string const& key,
                   JsonVariant const& config)
{
  try {
    this->components.at(key)(entity, config);
  } catch (std::out_of_range const&) {
    std::cerr << key << ": unknow component" << '\n';
  }
}
