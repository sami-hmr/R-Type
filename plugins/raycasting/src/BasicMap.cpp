#include "plugin/components/BasicMap.hpp"
#include "Raycasting.hpp"
#include "ecs/InitComponent.hpp"
#include "libs/Vector2D.hpp"

void Raycasting::init_basic_map(Registry::Entity& e, const JsonObject& obj)
{
  Vector2D size;
  if (!obj.contains("size")) {
    std::cerr << "[Raycasting] BasicMap component missing 'size' field"
              << std::endl;
    return;
  }
  size = get_value<BasicMap, Vector2D>(this->_registry.get(), obj, e, "size", "width", "height")
             .value();

  std::vector<std::vector<int>> data;
  if (!obj.contains("data")) {
    std::cerr << "[Raycasting] BasicMap component missing 'data' field"
              << std::endl;
    return;
  }
  JsonArray lines =
      get_value<BasicMap, JsonArray>(this->_registry.get(), obj, e, "data")
          .value();

  for (const auto& line : lines) {
    try {
      JsonArray col = std::get<JsonArray>(line.value);
      data.emplace_back();
      for (const auto& nb : col) {
        int nbr = std::get<int>(nb.value);
        data.back().push_back(nbr);
      }
    } catch (std::bad_variant_access& e) {
      std::cerr << "error parsing basic map, bad access tu connais"
                << std::endl;
      return;
    }
  }

  init_component<BasicMap>(
      this->_registry.get(), this->_event_manager.get(), e, size, data);
}