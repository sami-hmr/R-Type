#include "UI.hpp"

#include "ecs/EventManager.hpp"
#include "ecs/InitComponent.hpp"
#include "ecs/Registry.hpp"
#include "ecs/zipper/Zipper.hpp"
#include "libs/Color.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/components/AnimatedSprite.hpp"
#include "plugin/components/Background.hpp"
#include "plugin/components/Camera.hpp"
#include "plugin/components/Drawable.hpp"
#include "plugin/components/Input.hpp"
#include "plugin/components/Slider.hpp"
#include "plugin/components/Sprite.hpp"
#include "plugin/components/Text.hpp"
#include "plugin/events/IoEvents.hpp"

void UI::init_drawable(Ecs::Entity const& entity, JsonObject const& obj)
{
  bool enabled = true;
  if (obj.contains("enabled")) {
    enabled =
        get_value<Drawable, bool>(this->_registry.get(), obj, entity, "enabled")
            .value();
  }
  bool stretch = false;
  if (obj.contains("stretch")) {
    stretch =
        get_value<Drawable, bool>(this->_registry.get(), obj, entity, "stretch")
            .value();
  }
    init_component<Drawable>(
      this->_registry.get(), this->_event_manager.get(), entity,
      enabled, stretch);
}

void UI::init_sprite(Ecs::Entity const& entity, JsonObject const& obj)
{
  auto const& texture_path = get_value<Sprite, std::string>(
      this->_registry.get(), obj, entity, "texture");

  if (!texture_path) {
    std::cerr << "Error loading sprite component: unexpected value type "
                 "(texture: string)\n";
    return;
  }

  Vector2D scale(0.1, 0.1);
  if (obj.contains("size")) {
    scale = get_value<Sprite, Vector2D>(
                this->_registry.get(), obj, entity, "size", "width", "height")
                .value();
  }
  init_component<Sprite>(this->_registry.get(),
                         this->_event_manager.get(),
                         entity,
                         texture_path.value(),
                         scale);
}

void UI::init_text(Ecs::Entity const& entity, JsonObject const& obj)
{
  auto const& font_path =
      get_value<Text, std::string>(this->_registry.get(), obj, entity, "font");

  if (!font_path) {
    std::cerr << "Error loading text component: unexpected value type (font: "
                 "string)\n";
    return;
  }
  Vector2D scale(0.1, 0.1);
  if (obj.contains("size")) {
    scale = get_value<Text, Vector2D>(
                this->_registry.get(), obj, entity, "size", "width", "height")
                .value();
  }

  std::optional<std::string> str =
      get_value<Text, std::string>(this->_registry.get(), obj, entity, "text");

  if (!str.has_value()) {
    std::cerr << "Error loading text component: unexpected value type "
                 "(text: string)\n";
    return;
  }
  std::string text = std::move(str.value());

  const std::optional<Color>& outline_color = get_value<Text, Color>(
      this->_registry.get(), obj, entity, "outline_color");

  if (!outline_color.has_value()) {
    std::cerr << "Error loading text component: unexpected value type "
                 "(outline_color: Color)\n";
    return;
  }

  const std::optional<Color>& fill_color =
      get_value<Text, Color>(this->_registry.get(), obj, entity, "fill_color");
  if (!fill_color.has_value()) {
    std::cerr << "Error loading text component: unexpected value type "
                 "(fill_color: Color)\n";
    return;
  }

  const std::optional<bool>& outline =
      get_value<Text, bool>(this->_registry.get(), obj, entity, "outline");
  if (!outline.has_value()) {
    std::cerr << "Error loading text component: unexpected value type "
                 "(outline: bool)\n";
    return;
  }

  const std::optional<double>& outline_thickness = get_value<Text, double>(
      this->_registry.get(), obj, entity, "outline_thickness");
  if (!outline_thickness.has_value()) {
    std::cerr << "Error loading text component: unexpected value type "
                 "(outline_thickness: double)\n";
    return;
  }

  std::string placeholder;
  if (obj.contains("placeholder")) {
    placeholder = get_value<Text, std::string>(
                      this->_registry.get(), obj, entity, "placeholder")
                      .value();
  }

  init_component(this->_registry.get(),
                 this->_event_manager.get(),
                 entity,
                 Text(font_path.value(),
                      scale,
                      text,
                      placeholder,
                      outline_color.value(),
                      fill_color.value(),
                      outline.value(),
                      outline_thickness.value()));
}

void UI::init_input(Ecs::Entity entity, const JsonVariant& config)
{
  bool enabled = false;
  std::string buffer;

  try {
    JsonObject obj = std::get<JsonObject>(config);
    if (obj.contains("enabled")) {
      enabled = std::get<bool>(obj.at("enabled").value);
    }
    if (obj.contains("buffer")) {
      buffer = std::get<std::string>(obj.at("buffer").value);
    }

    init_component<Input>(this->_registry.get(),
                          this->_event_manager.get(),
                          entity,
                          Input(enabled, buffer));
  } catch (std::bad_variant_access const&) {
  }
}

void UI::handle_key_pressed(const KeyPressedEvent& event)
{
  auto& inputs = _registry.get().get_components<Input>();

  for (auto& input : inputs) {
    if (!input.has_value()) {
      continue;
    }

    if (!input.value().enabled) {
      continue;
    }

    if (event.key_unicode.has_value()) {
      input.value().buffer += event.key_unicode.value();
    }

    if (event.key_pressed.contains(Key::DELETE)
        && event.key_pressed.at(Key::DELETE))
    {
      if (!input.value().buffer.empty()) {
        input.value().buffer.pop_back();
      }
    }
  }
}

void UI::init_background(Ecs::Entity const& entity, JsonObject const& obj)
{
  auto const& textures_path = get_value<Background, JsonArray>(
      this->_registry.get(), obj, entity, "layers");

  if (!textures_path) {
    std::cerr << "Error loading Background component: unexpected value type "
                 "(expected texture: string)\n";
    return;
  }
  std::vector<std::string> paths;

  for (const auto& layer : textures_path.value()) {
    JsonObject path_obj;
    try {
      path_obj = std::get<JsonObject>(layer.value);
    } catch (std::bad_variant_access const&) {
      std::cerr << "Error loading Background component: unexpected layer "
                     "value type (expected JsonObject)\n";
      continue;
    }
    const auto& path_str = get_value<Background, std::string>(
        this->_registry.get(), path_obj, entity, "path");

    if (path_str.has_value()) {
      paths.push_back(path_str.value());
    }
  }
  Background::RenderType render_type = Background::RenderType::NOTHING;

  auto const& render_type_str = get_value<Background, std::string>(
      this->_registry.get(), obj, entity, "render_type");
  if (render_type_str.has_value()) {
    if (render_type_map.contains(render_type_str.value())) {
      render_type = render_type_map.at(render_type_str.value());
    } else {
      std::cerr << "Error loading Background component: invalid render_type "
                   "value, using default (NOTHING)\n";
    }
  }
  Parallax parallax;
  const std::optional<JsonObject>& parallax_obj =
      get_value<Background, JsonObject>(
          this->_registry.get(), obj, entity, "parallax");

  if (parallax_obj.has_value()) {
    const auto& active = get_value<Background, bool>(
        this->_registry.get(), parallax_obj.value(), entity, "active");
    const auto& speed = get_value<Background, Vector2D>(
        this->_registry, parallax_obj.value(), entity, "speed");
    const auto& framerate = get_value<Background, double>(
        this->_registry.get(), parallax_obj.value(), entity, "framerate");

    if (active.has_value() && framerate.has_value() && speed.has_value()) {
      parallax.speed = Vector2D(speed.value());
      parallax.active = active.value();
      parallax.framerate = framerate.value();
    } else {
      std::cerr << "Error loading Background component: invalid parallax "
                   "value, using default (inactive)\n";
    }
  }
  init_component<Background>(this->_registry.get(),
                             this->_event_manager.get(),
                             entity,
                             Background(paths, render_type, parallax));
}
