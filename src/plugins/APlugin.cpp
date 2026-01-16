#include <iostream>
#include <vector>

#include "plugin/APlugin.hpp"

#include "Json/JsonParser.hpp"

APlugin::APlugin(
    std::string name,
    Registry& registry,
    EventManager& event_manager,
    EntityLoader& loader,
    std::vector<std::string> const& depends_on,
    std::unordered_map<
        std::string,
        std::function<void(Registry::Entity, JsonVariant const&)>> components,
    std::optional<JsonObject> const& config)
    : name(std::move(name))
    , components(std::move(components))
    , _registry(registry)
    , _event_manager(event_manager)
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
  } catch (std::out_of_range const& e) {
    std::cerr << key << ": unknown component with config: " << e.what() << '\n';
    // for (auto i : std::get<JsonObject>(config)) {
    //   std::cout << i.first << std::endl;
    // }
  }
}
