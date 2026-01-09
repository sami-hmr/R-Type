#include "plugin/components/Slider.hpp"

#include "Json/JsonParser.hpp"
#include "UI.hpp"
#include "ecs/InitComponent.hpp"
#include "ecs/Registry.hpp"
#include "ecs/zipper/Zipper.hpp"
#include "libs/Color.hpp"
#include "libs/Rect.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/components/Drawable.hpp"
#include "plugin/components/Position.hpp"
#include "plugin/events/IoEvents.hpp"

void UI::init_slider(const Registry::Entity& e, const JsonObject& obj)
{
  Vector2D size;
  if (!obj.contains("size")) {
    std::cerr << "Error loading Slider component: unexpected value type "
                 "(size: Vector2D)\n";
    return;
  }
  size = get_value<Slider, Vector2D>(this->_registry.get(), obj, e, "size", "width", "height")
             .value();
  Color bar_color = WHITE;
  if (obj.contains("bar_color")) {
    auto color_opt =
        get_value<Slider, Color>(this->_registry.get(), obj, e, "bar_color");
    if (color_opt) {
      bar_color = color_opt.value();
    }
  }
  Color circle_color = RED;
  if (obj.contains("circle_color")) {
    auto color_opt = get_value<Slider, Color>(this->_registry.get(), obj, e, "circle_color");
    if (color_opt) {
      circle_color = color_opt.value();
    }
  }
  auto min_value =
      get_value<Slider, double>(this->_registry.get(), obj, e, "min_value");
  if (!min_value) {
    std::cerr << "Error loading Slider component: unexpected value type "
                 "(min_value: double)\n";
    return;
  }
  auto max_value =
      get_value<Slider, double>(this->_registry.get(), obj, e, "max_value");
  if (!max_value) {
    std::cerr << "Error loading Slider component: unexpected value type "
                 "(max_value: double)\n";
    return;
  }
  auto current_value =
      get_value<Slider, double>(this->_registry.get(), obj, e, "current_value");
  if (!current_value) {
    std::cerr << "Error loading Slider component: unexpected value type "
                 "(current_value: double)\n";
    return;
  }
  auto step = get_value<Slider, double>(this->_registry.get(), obj, e, "step");
  if (!step) {
    std::cerr << "Error loading Slider component: unexpected value type "
                 "(step: double)\n";
    return;
  }
  bool vertical = false;
  if (obj.contains("vertical")) {
    auto vertical_opt =
        get_value<Slider, bool>(this->_registry.get(), obj, e, "vertical");
    if (vertical_opt) {
      vertical = vertical_opt.value();
    }
  }
  init_component<Slider>(this->_registry.get(),
                         this->_event_manager.get(),
                         e,
                         size,
                         bar_color,
                         circle_color,
                         min_value.value(),
                         max_value.value(),
                         current_value.value(),
                         step.value(),
                         false,
                         vertical);
}

void on_click_slider(Registry& r, const MousePressedEvent& event)
{
  for (auto&& [draw, slider, pos] : Zipper<Drawable, Slider, Position>(r)) {
    if (!draw.enabled) {
      continue;
    }
    Rect entity_rect = {.x = pos.pos.x,
                        .y = pos.pos.y,
                        .width = slider.size.x * 2.0,
                        .height = slider.size.y * 2.0};
    slider.selected = entity_rect.contains(event.position.x, event.position.y);
  }
}

void on_release_slider(Registry& r, const MouseReleasedEvent& /*event*/)
{
  for (auto&& [slider] : Zipper<Slider>(r)) {
    slider.selected = false;
  }
}
