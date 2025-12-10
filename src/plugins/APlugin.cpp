#include <iostream>
#include <vector>

#include "plugin/APlugin.hpp"

#include "Json/JsonParser.hpp"

APlugin::APlugin(
    Registry& registry,
    EntityLoader& loader,
    std::vector<std::string> const& depends_on,
    std::unordered_map<
        std::string,
        std::function<void(Registry::Entity, JsonVariant const&)>> components,
    std::optional<JsonObject> const& config)
    : components(std::move(components))
    , _registry(registry)
    , _loader(loader)
    , _config(config)
{
  for (auto const& p : depends_on) {
    this->_loader.get().load_plugin(p);
  }
}

void APlugin::set_component(Registry::Entity entity,
                            std::string const& key,
                            JsonVariant const& config)
{
  try {
    this->components.at(key)(entity, config);
  } catch (std::out_of_range const&) {
    std::cerr << key << ": unknown component with config: " << '\n';
    for (auto i : std::get<JsonObject>(config)) {
      std::cout << i.first << std::endl;
    }
  }
}
