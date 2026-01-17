#include "plugin/components/Bar.hpp"

#include "ATH.hpp"
#include "Json/JsonParser.hpp"
#include "ecs/InitComponent.hpp"
#include "ecs/Registry.hpp"
#include "ecs/zipper/Zipper.hpp"
#include "ecs/zipper/ZipperIndex.hpp"
#include "plugin/Hooks.hpp"

void ATH::init_bar(Ecs::Entity& e, const JsonObject& obj)
{
  auto size = get_value<Bar, Vector2D>(this->_registry.get(), obj, e, "size");
  if (!size) {
    std::cerr << "Error loading Bar component: unexpected value type (size: "
                 "Vector2D)\n";
    return;
  }
  auto max_value =
      get_value<Bar, double>(this->_registry.get(), obj, e, "max_value");
  if (!max_value) {
    std::cerr << "Error loading Bar component: unexpected value type "
                 "(max_value: double)\n";
    return;
  }
  auto current_value =
      get_value<Bar, double>(this->_registry.get(), obj, e, "current_value");
  if (!current_value) {
    std::cerr << "Error loading Bar component: unexpected value type "
                 "(current_value: double)\n";
    return;
  }
  Vector2D offset(0, 0);
  if (obj.contains("offset")) {
    auto offset_opt =
        get_value<Bar, Vector2D>(this->_registry.get(), obj, e, "offset");
    if (offset_opt) {
      offset = offset_opt.value();
    }
  }
  Color color = WHITE;
  if (obj.contains("color")) {
    auto color_opt =
        get_value<Bar, Color>(this->_registry.get(), obj, e, "color");
    if (color_opt) {
      color = color_opt.value();
    }
  }
  std::string texture_path;
  if (obj.contains("texture_path")) {
    auto texture_path_opt = get_value<Bar, std::string>(
        this->_registry.get(), obj, e, "texture_path");
    if (texture_path_opt) {
      texture_path = texture_path_opt.value();
    }
  }
  bool outline = false;
  if (obj.contains("outline")) {
    auto outline_opt =
        get_value<Bar, bool>(this->_registry.get(), obj, e, "outline");
    if (outline_opt) {
      outline = outline_opt.value();
    }
  }
  init_component<Bar>(this->_registry.get(),
                      this->_event_manager.get(),
                      e,
                      size.value(),
                      max_value.value(),
                      current_value.value(),
                      offset,
                      color,
                      texture_path,
                      outline);
}
