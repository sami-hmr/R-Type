#include "UI.hpp"

#include "libs/Vector2D.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/components/AnimatedSprite.hpp"
#include "plugin/components/Background.hpp"
#include "plugin/components/Camera.hpp"
#include "plugin/components/Drawable.hpp"
#include "plugin/components/Input.hpp"
#include "plugin/components/Sprite.hpp"
#include "plugin/components/Text.hpp"

UI::UI(Registry& r, EntityLoader& l, std::optional<JsonObject> const& config)
    : APlugin("ui",
              r,
              l,
              {},
              {COMP_INIT(input, Input, init_input),
               COMP_INIT(Drawable, Drawable, init_drawable),
               COMP_INIT(Sprite, Sprite, init_sprite),
               COMP_INIT(Text, Text, init_text),
               COMP_INIT(Camera, Camera, init_cam),
               COMP_INIT(Background, Background, init_background),
               COMP_INIT(AnimatedSprite, AnimatedSprite, init_animated_sprite)},
              config)
{
  SUBSCRIBE_EVENT(KeyPressedEvent, { this->handle_key_pressed(event); })

  REGISTER_COMPONENT(Drawable)
  REGISTER_COMPONENT(Input)
  REGISTER_COMPONENT(Sprite)
  REGISTER_COMPONENT(Text)
  REGISTER_COMPONENT(Camera)
  REGISTER_COMPONENT(Background)
  REGISTER_COMPONENT(AnimatedSprite)
}

void UI::init_drawable(Registry::Entity const& entity, JsonObject const&)
{
  _registry.get().emplace_component<Drawable>(entity);
}

void UI::init_sprite(Registry::Entity const& entity, JsonObject const& obj)
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
  _registry.get().emplace_component<Sprite>(
      entity, texture_path.value(), scale);
}

void UI::init_text(Registry::Entity const& entity, JsonObject const& obj)
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

  auto& text_opt = _registry.get().emplace_component<Text>(
      entity, font_path.value(), scale, "");

  if (text_opt.has_value()) {
    auto text_val = get_value<Text, std::string>(
        this->_registry.get(), obj, entity, "text");
    if (text_val) {
      text_opt.value().text = text_val.value();
    }
  }
}

void UI::init_input(Registry::Entity entity, const JsonVariant& config)
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

    _registry.get().emplace_component<Input>(entity, Input(enabled, buffer));
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

void UI::init_background(Registry::Entity const& entity, JsonObject const& obj)
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
      std::cout << "Adding background layer: " << path_str.value() << "\n";
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
  this->_registry.get().emplace_component<Background>(
      entity, Background(paths, render_type, parallax));
}

std::optional<AnimationData> UI::parse_animation_data(JsonObject const& obj,
                                                      Registry::Entity const& e)
{
  AnimationData animdata;

  auto const& texture_path = get_value<AnimatedSprite, std::string>(
      this->_registry.get(), obj, e, "texture");
  if (!texture_path) {
    std::cerr << "Error parsing animation data: \"texture\" field not "
                 "found or invalid"
              << "\n";
    return std::nullopt;
  }
  animdata.texture_path = texture_path.value();

  animdata.frame_size = get_value<AnimatedSprite, Vector2D>(
                            this->_registry.get(), obj, e, "frame_size")
                            .value();

  animdata.frame_pos = get_value<AnimatedSprite, Vector2D>(
                           this->_registry.get(), obj, e, "frame_pos")
                           .value();

  auto const& framerate = get_value<AnimatedSprite, double>(
      this->_registry.get(), obj, e, "framerate");
  if (!framerate) {
    std::cerr << "Error parsing animation data: \"framerate\" field not found "
                 "or invalid"
              << "\n";
    return std::nullopt;
  }
  animdata.direction = get_value<AnimatedSprite, Vector2D>(
                           this->_registry.get(), obj, e, "direction")
                           .value();

  animdata.framerate = framerate.value();
  animdata.sprite_size =
      get_value<Text, Vector2D>(
          this->_registry.get(), obj, e, "sprite_size", "width", "height")
          .value();

  auto const& nb_frames = get_value<AnimatedSprite, int>(
      this->_registry.get(), obj, e, "nb_frames");
  if (!nb_frames) {
    std::cerr << "Error parsing animation data: \"nb_frames\" field not found "
                 "or invalid"
              << "\n";
    return std::nullopt;
  }
  animdata.nb_frames = nb_frames.value();
  auto const& loop =
      get_value<AnimatedSprite, bool>(this->_registry.get(), obj, e, "loop");
  if (!loop) {
    std::cerr
        << "Error parsing animation data: \"loop\" field not found or invalid"
        << "\n";
    return std::nullopt;
  }
  animdata.loop = loop.value();
  auto const& rollback = get_value<AnimatedSprite, bool>(
      this->_registry.get(), obj, e, "rollback");
  if (!rollback) {
    std::cerr << "Error parsing animation data: \"rollback\" field not found "
                 "or " "invalid"
              << "\n";
    return std::nullopt;
  }
  animdata.rollback = rollback.value();
  return animdata;
}

void UI::init_animated_sprite(Registry::Entity const& entity,
                              const JsonObject& obj)
{
  std::unordered_map<std::string, AnimationData> animations;

  std::optional<JsonArray> animations_obj =
      get_value<AnimatedSprite, JsonArray>(
          this->_registry.get(), obj, entity, "animations");

  if (!animations_obj) {
    std::cerr << "AnimatedSprite component requires animations array"
              << "\n";
    return;
  }
  for (const JsonValue& animation_value : animations_obj.value()) {
    std::optional<AnimationData> animdata;
    std::string name;
    JsonObject animdata_obj;
    try {
      animdata_obj = std::get<JsonObject>(animation_value.value);
    } catch (std::bad_variant_access const&) {
      std::cerr << "Error parsing animation data: not a JsonObject" << '\n';
      return;
    }
    if (!animdata_obj.contains("name")) {
      std::cerr << "Error parsing animation data: \"name\" field not found"
                << '\n';
      return;
    }
    try {
      name = std::get<std::string>(animdata_obj.at("name").value);
    } catch (std::bad_variant_access const&) {
      std::cerr
          << "Error parsing animation data: \"name\" field is not a string"
          << '\n';
      return;
    }
    animdata = parse_animation_data(animdata_obj, entity);

    if (!animdata) {
      std::cerr << "Error parsing animation data for animation: " << name
                << '\n';
      return;
    }
    animations.insert_or_assign(name, animdata.value());
  }
  std::string default_animation = animations.begin()->first;
  auto const& default_animation_value = get_value<AnimatedSprite, std::string>(
      this->_registry.get(), obj, entity, "default_animation");
  if (default_animation_value) {
    default_animation = default_animation_value.value();
  }
  _registry.get().emplace_component<AnimatedSprite>(
      entity, std::move(animations), default_animation, default_animation);
}

void UI::init_cam(Registry::Entity const& entity, JsonObject const& obj)
{
  Vector2D size(0.5, 0.5);
  Vector2D target(0.0, 0.0);
  Vector2D speed(0.1, 0.1);

  auto sizeopt = get_value<Camera, Vector2D>(
      this->_registry.get(), obj, entity, "size", "width", "height");
  if (sizeopt.has_value()) {
    size = sizeopt.value();
  } else {
    std::cerr
        << "Camera component missing size field, using default (50%, 50%)\n";
    return;
  }
  auto targetopt =
      get_value<Camera, Vector2D>(this->_registry.get(), obj, entity, "target");
  if (targetopt.has_value()) {
    target = targetopt.value();
  } else {
    std::cerr
        << "Camera component missing target field, using default (0, 0)\n";
    return;
  }
  auto speedopt = get_value<Camera, Vector2D>(
      this->_registry.get(), obj, entity, "speed", "x", "y");
  if (speedopt.has_value()) {
    speed = speedopt.value();
  } else {
    std::cerr
        << "Camera component missing speed field, using default (10%, 15%)\n";
    return;
  }
  _registry.get().emplace_component<Camera>(entity, size, target, speed);
  _registry.get().on<CamAggroEvent>("CamAggroEvent",
                                    [this](const CamAggroEvent& e)
                                    { this->cam_target_event(e); });
  _registry.get().on<CamZoomEvent>("CamZoomEvent",
                                   [this](const CamZoomEvent& e)
                                   { this->cam_zoom_event(e); });
  _registry.get().on<CamRotateEvent>("CamRotateEvent",
                                     [this](const CamRotateEvent& e)
                                     { this->cam_rotate_event(e); });
  _registry.get().on<CamSpeedEvent>("CamSpeedEvent",
                                    [this](const CamSpeedEvent& e)
                                    { this->cam_speed_event(e); });
}

extern "C"
{
void* entry_point(Registry& r,
                  EntityLoader& l,
                  std::optional<JsonObject> const& config)
{
  return new UI(r, l, config);
}
}
