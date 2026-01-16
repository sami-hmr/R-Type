
#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <numbers>
#include <cstdint>
#include <cstdio>
#include <functional>
#include <iostream>
#include <locale>
#include <stdexcept>
#include <tuple>

#include "SFMLRenderer.hpp"

#include <SFML/Audio/Music.hpp>
#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/System/Angle.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Cursor.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>

#include "Drawable.hpp"
#include "ecs/EventManager.hpp"
#include "ecs/Registry.hpp"
#include "ecs/Scenes.hpp"
#include "ecs/SparseArray.hpp"
#include "ecs/zipper/Zipper.hpp"
#include "ecs/zipper/ZipperIndex.hpp"
#include "libs/Rect.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/components/AnimatedSprite.hpp"
#include "plugin/components/Background.hpp"
#include "plugin/components/Button.hpp"
#include "plugin/components/Camera.hpp"
#include "plugin/components/Clickable.hpp"
#include "plugin/components/Collidable.hpp"
#include "plugin/components/Drawable.hpp"
#include "plugin/components/Facing.hpp"
#include "plugin/components/Input.hpp"
#include "plugin/components/Position.hpp"
#include "plugin/components/Sprite.hpp"
#include "plugin/components/Text.hpp"
#include "plugin/events/AnimationEvents.hpp"
#include "plugin/events/DamageEvent.hpp"
#include "plugin/events/DeathEvent.hpp"
#include "plugin/events/IoEvents.hpp"
#include "plugin/events/LoggerEvent.hpp"
#include "plugin/events/ShutdownEvent.hpp"

static sf::Texture gen_placeholder()
{
  sf::Image image(SFMLRenderer::placeholder_size, sf::Color::Black);

  for (unsigned int y = 0; y < SFMLRenderer::placeholder_size.y; ++y) {
    for (unsigned int x = 0; x < SFMLRenderer::placeholder_size.x; ++x) {
      if ((x + y) % 2 == 0) {
        image.setPixel({x, y}, sf::Color::Magenta);
      }
    }
  }
  return sf::Texture(image);
}

static sf::SoundBuffer gen_sound_placeholder()
{
  const unsigned int SAMPLE_RATE = 44100;
  const unsigned int DURATION = 1;
  const unsigned int SAMPLE_COUNT = SAMPLE_RATE * DURATION;

  std::vector<std::int16_t> samples(SAMPLE_COUNT);

  const double FREQUENCY = 440.0;
  const double amplitude = 10000.0;

  for (unsigned int i = 0; i < SAMPLE_COUNT; ++i) {
    double time = static_cast<double>(i) / SAMPLE_RATE;
    samples[i] = static_cast<std::int16_t>(
        amplitude * std::sin(2.0 * std::numbers::pi * FREQUENCY * time));
  }
  sf::SoundBuffer sound_buffer;
  if (!sound_buffer.loadFromSamples(samples.data(),
                                    SAMPLE_COUNT,
                                    1,
                                    SAMPLE_RATE,
                                    {sf::SoundChannel::Mono}))
  {
    return {};
  }
  return sound_buffer;
}

SFMLRenderer::SFMLRenderer(Registry& r, EventManager& em, EntityLoader& l)
    : APlugin("sfml", r, em, l, {"moving", "ath", "ui", "collision", "sound"}, {})
{
  _window =
      sf::RenderWindow(sf::VideoMode(window_size), "R-Type - SFML Renderer");
  _window.setFramerateLimit(window_rate);

  _registry.get().add_system([this](Registry&) { this->handle_events(); }, 1);
  _registry.get().add_system([this](Registry& r) { this->camera_system(r); });
  _registry.get().add_system([this](Registry&)
                             { _window.clear(sf::Color::Black); });
  _registry.get().add_system([this](Registry& r)
                             { this->background_system(r); });

  _registry.get().add_system([this](Registry& r) { this->button_system(r); });
  _registry.get().add_system([this](Registry& r) { this->slider_system(r); });
  _registry.get().add_system([this](Registry& r)
                             { this->unified_render_system(r); });
  _registry.get().add_system([this](Registry&) { this->display(); });
  _registry.get().add_system([this](Registry& r) { this->sounds_system(r); });
  _registry.get().add_system([this](Registry& r) { this->musics_system(r); });
  _registry.get().add_system([this](Registry& r) { this->hover_system(r); });

  _textures.insert_or_assign(SFMLRenderer::placeholder, gen_placeholder());
  _sound_buffers.insert_or_assign(SFMLRenderer::placeholder,
                                  gen_sound_placeholder());

  SUBSCRIBE_EVENT_PRIORITY(
      MousePressedEvent, { this->on_click(event); }, 10000);
  SUBSCRIBE_EVENT(InputFocusEvent, { this->on_input_focus(event); });
  sf::SoundBuffer& buffer = _sound_buffers.at(SFMLRenderer::placeholder);
  for (auto& sound : this->_sounds) {
    sound = sf::Sound(buffer);
  }
  this->_musics.insert_or_assign(SFMLRenderer::placeholder, sf::Music());
  this->_cursors.insert_or_assign("arrow", sf::Cursor(sf::Cursor::Type::Arrow));
  this->_cursors.insert_or_assign("hand", sf::Cursor(sf::Cursor::Type::Hand));
  this->_cursors.insert_or_assign("wait", sf::Cursor(sf::Cursor::Type::Wait));
  this->_cursors.insert_or_assign("text", sf::Cursor(sf::Cursor::Type::Text));
}

SFMLRenderer::~SFMLRenderer()
{
  if (_window.isOpen()) {
    _window.close();
  }
}

sf::Texture& SFMLRenderer::load_texture(std::string const& path)
{
  if (_textures.contains(path)) {
    return _textures.at(path);
  }
  sf::Texture texture;
  if (!texture.loadFromFile(path)) {
    LOGGER("SFML", LogLevel::ERR, "Failed to load texture: " + path)
    return _textures.at(placeholder);
  }
  _textures.insert_or_assign(path, std::move(texture));
  return _textures.at(path);
}

sf::Font& SFMLRenderer::load_font(std::string const& path)
{
  if (_fonts.contains(path)) {
    return _fonts.at(path);
  }
  sf::Font font;
  if (!font.openFromFile(path)) {
    LOGGER("SFML", LogLevel::ERR, "Failed to load font: " + path)
    throw std::runtime_error("Failed to load font: " + path);
  }
  _fonts.insert_or_assign(path, std::move(font));
  return _fonts.at(path);
}

std::optional<Key> SFMLRenderer::sfml_key_to_key(sf::Keyboard::Key sfml_key)
{
  auto it = key_association.find(sfml_key);
  if (it != key_association.end()) {
    return it->second;
  }
  return std::nullopt;
}

void SFMLRenderer::on_input_focus(const InputFocusEvent& /*unused*/)
{
  this->_window.setMouseCursor(this->_cursors.at("text"));
}

void SFMLRenderer::on_click(const MousePressedEvent& /*unused*/)
{
  this->_window.setMouseCursor(this->_cursors.at("arrow"));
}

void SFMLRenderer::handle_resize()
{
  sf::Vector2u new_size = _window.getSize();
  this->_view.setSize(sf::Vector2f(static_cast<float>(new_size.x),
                                   static_cast<float>(new_size.y)));
  this->_view.setCenter(sf::Vector2f(static_cast<float>(new_size.x) / 2,
                                     static_cast<float>(new_size.y) / 2));
  _window.setView(this->_view);
}

static constexpr double deux =
    2.0;  // allez le linter t content mtn y'a une constante

Vector2D SFMLRenderer::screen_to_world(sf::Vector2i screen_pos)
{
  sf::Vector2f world_pos = _window.mapPixelToCoords(screen_pos, _view);
  sf::Vector2u window_size = _window.getSize();
  auto min_dimension =
      static_cast<float>(std::min(window_size.x, window_size.y));
  sf::Vector2f offset(
      (static_cast<float>(window_size.x) - min_dimension) / 2.0f,
      (static_cast<float>(window_size.y) - min_dimension) / 2.0f);

  return {((world_pos.x - offset.x) * deux / min_dimension) - 1.0,
          ((world_pos.y - offset.y) * deux / min_dimension) - 1.0};
}

void SFMLRenderer::mouse_events(const sf::Event& events)
{
  const sf::Vector2i mouse_pos = sf::Mouse::getPosition(_window);
  const auto* mouse_pressed = events.getIf<sf::Event::MouseButtonPressed>();
  const auto* mouse_released = events.getIf<sf::Event::MouseButtonReleased>();

  this->_mouse_pos = screen_to_world(mouse_pos);
  if (mouse_pressed != nullptr) {
    if (MOUSEBUTTONMAP.contains(mouse_pressed->button)) {
      MouseButton button = MOUSEBUTTONMAP.at(mouse_pressed->button);
      MousePressedEvent mouse_event(this->_mouse_pos, button);
      this->_event_manager.get().emit<MousePressedEvent>(mouse_event);
    }
  }
  if (mouse_released != nullptr) {
    if (MOUSEBUTTONMAP.contains(mouse_released->button)) {
      MouseButton button = MOUSEBUTTONMAP.at(mouse_released->button);
      MouseReleasedEvent mouse_event(this->_mouse_pos, button);
      this->_event_manager.get().emit<MouseReleasedEvent>(mouse_event);
    }
  }
}

void SFMLRenderer::handle_events()
{
  if (!_window.isOpen()) {
    return;
  }

  _key_pressed.key_pressed.clear();
  if (_key_pressed.key_unicode.has_value()) {
    _key_pressed.key_unicode.reset();
  }

  _key_released.key_released.clear();
  if (_key_released.key_unicode.has_value()) {
    _key_released.key_unicode.reset();
  }

  while (const std::optional event = _window.pollEvent()) {
    if (event->is<sf::Event::Closed>()) {
      _window.close();
      _event_manager.get().emit<ShutdownEvent>("Window closed", 0);
    }
    this->mouse_events(event.value());
    if (const auto* key_pressed = event->getIf<sf::Event::KeyPressed>()) {
      auto key = sfml_key_to_key(key_pressed->code);
      if (key.has_value()) {
        _key_pressed.key_pressed[key.value()] = true;
      }
    }
    if (const auto* key_released = event->getIf<sf::Event::KeyReleased>()) {
      auto key = sfml_key_to_key(key_released->code);
      if (key.has_value()) {
        _key_released.key_released[key.value()] = true;
      }
    }
    if (const auto* text_entered = event->getIf<sf::Event::TextEntered>()) {
      if (std::isalnum(text_entered->unicode) || text_entered->unicode == ' ') {
        if (!_key_pressed.key_unicode.has_value()) {
          _key_pressed.key_unicode = "";
        }
        _key_pressed.key_unicode.value() +=
            static_cast<char>(text_entered->unicode);
      }
    }
    if (event->is<sf::Event::Resized>()) {
      handle_resize();
    }
  }
  if (!_key_pressed.key_pressed.empty() || _key_pressed.key_unicode.has_value())
  {
    _event_manager.get().emit<KeyPressedEvent>(_key_pressed);
  }
  if (!_key_released.key_released.empty()) {
    _event_manager.get().emit<KeyReleasedEvent>(_key_released);
  }
}

void SFMLRenderer::display()
{
  if (!_window.isOpen()) {
    return;
  }
  _window.display();
}

void SFMLRenderer::render_sprites(Registry& r,
                                  std::vector<DrawableItem>& all_drawables,
                                  float min_dimension,
                                  const sf::Vector2u& window_size,
                                  const sf::Vector2f& view_size,
                                  const sf::Vector2f& view_pos)
{
  for (auto&& [pos, draw, spr] : Zipper<Position, Drawable, Sprite>(r)) {
    if (!draw.enabled) {
      continue;
    }

    float offset_x = (window_size.x - min_dimension) / 2.0f;
    float offset_y = (window_size.y - min_dimension) / 2.0f;

    sf::Vector2f new_pos(
        static_cast<float>((pos.pos.x + 1.0) * min_dimension / deux) + offset_x,
        static_cast<float>((pos.pos.y + 1.0) * min_dimension / deux)
            + offset_y);

    if (new_pos.x < view_pos.x - (view_size.x / 2)
        || new_pos.x > view_pos.x + (view_size.x / 2))
    {
      continue;
    }
    if (new_pos.y < view_pos.y - (view_size.y / 2)
        || new_pos.y > view_pos.y + (view_size.y / 2))
    {
      continue;
    }
    sf::Texture& texture = load_texture(spr.texture_path);

    float scale_x =
        static_cast<float>(min_dimension * spr.scale.x) / texture.getSize().x;
    float scale_y =
        static_cast<float>(min_dimension * spr.scale.y) / texture.getSize().y;
    float uniform_scale = std::min(scale_x, scale_y);

    if (!this->_sprite.has_value()) {
      this->_sprite = sf::Sprite(texture);
    }

    draw.true_size = Vector2D(
        std::max(static_cast<double>(texture.getSize().x * uniform_scale)
                     / min_dimension,
                 draw.true_size.x),
        std::max(static_cast<double>(texture.getSize().y * uniform_scale)
                     / min_dimension,
                 draw.true_size.y));

    draw.true_size =
        Vector2D {static_cast<double>(texture.getSize().x * uniform_scale)
                      / min_dimension,
                  static_cast<double>(texture.getSize().y * uniform_scale)
                      / min_dimension};

    SpriteDrawable sprite_drawable(std::ref(*this->_sprite),
                                   spr.texture_path,
                                   new_pos,
                                   sf::Vector2f(uniform_scale, uniform_scale),
                                   0.0f,
                                   pos.z);

    all_drawables.emplace_back(DrawableVariant {std::move(sprite_drawable)},
                               pos.z);
  }
}

void SFMLRenderer::render_texts(Registry& r,
                                std::vector<DrawableItem>& all_drawables,
                                float min_dimension,
                                const sf::Vector2u& window_size)
{
  for (auto&& [i, pos, draw, txt] : ZipperIndex<Position, Drawable, Text>(r)) {
    if (!draw.enabled) {
      continue;
    }

    std::string text_str = txt.text;
    auto fill_color = txt.fill_color;
    if (txt.text.empty()) {
      text_str = txt.placeholder;
      fill_color.a /= 2;
    }

    sf::Font& font = load_font(txt.font_path);
    if (!_text.has_value()) {
      _text = sf::Text(font);
    }

    constexpr unsigned int base_size = 100;
    _text.value().setFont(font);
    _text.value().setCharacterSize(base_size);
    sf::Rect<float> text_rect;

    _text.value().setString(
        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");  // caluculate height with all possible letters
    text_rect = _text.value().getLocalBounds();
    double min_dim = std::min(window_size.x, window_size.y);
    double desired_height = min_dim * txt.scale.y;
    double scale_y = desired_height / text_rect.size.y;

    _text.value().setString(text_str);
    text_rect = _text.value().getLocalBounds();
    double desired_width = min_dim * txt.scale.x;
    double scale_x = desired_width / text_rect.size.x;
    double text_scale = std::min(scale_x, scale_y);

    auto final_size = static_cast<unsigned int>(base_size * text_scale);

    float offset_x = (window_size.x - min_dimension) / deux;
    float offset_y = (window_size.y - min_dimension) / deux;

    sf::Vector2f new_pos(
        static_cast<float>((pos.pos.x + 1.0) * min_dimension / deux) + offset_x,
        static_cast<float>((pos.pos.y + 1.0) * min_dimension / deux)
            + offset_y);

    _text.value().setString(text_str);
    _text.value().setCharacterSize(final_size);
    sf::FloatRect final_text_rect = _text.value().getLocalBounds();

    draw.true_size = Vector2D {
        std::max(static_cast<double>(final_text_rect.size.x) / min_dimension,
                 draw.true_size.x),
        std::max(static_cast<double>(final_text_rect.size.y) / min_dimension,
                 draw.true_size.y)};

    TextDrawable text_drawable(
        std::ref(this->_text.value()),
        txt.font_path,
        text_str,
        new_pos,
        fill_color,
        txt.outline_color,
        txt.outline ? txt.outline_thickness * 0.1f : 0.0f,
        0.0f,
        pos.z,
        final_size,
        txt.outline);

    all_drawables.emplace_back(DrawableVariant {std::move(text_drawable)},
                               pos.z);
  }
}

void SFMLRenderer::render_bars(Registry& r,
                               std::vector<DrawableItem>& all_drawables,
                               float min_dimension,
                               const sf::Vector2u& window_size)
{
  for (auto&& [drawable, position, bar] : Zipper<Drawable, Position, Bar>(r)) {
    if (!drawable.enabled) {
      continue;
    }

    float offset_x = (window_size.x - min_dimension) / 2.0f;
    float offset_y = (window_size.y - min_dimension) / 2.0f;

    sf::Vector2f new_pos(
        static_cast<float>((position.pos.x + 1.0) * min_dimension / 2.0f)
            + offset_x,
        static_cast<float>((position.pos.y + 1.0) * min_dimension / 2.0f)
            + offset_y);
    sf::Vector2f size(static_cast<float>(bar.size.x * min_dimension),
                      static_cast<float>(bar.size.y * min_dimension));
    sf::Vector2f offset(static_cast<float>(bar.offset.x * min_dimension),
                        static_cast<float>(bar.offset.y * min_dimension));

    float fill_percentage = bar.current_value / bar.max_value;
    if (fill_percentage < 0.0f) {
      fill_percentage = 0.0f;
    } else if (fill_percentage > 1.0f) {
      fill_percentage = 1.0f;
    }

    std::optional<sf::Texture> texture_ptr = std::nullopt;
    if (bar.texture_path != "") {
      texture_ptr = load_texture(bar.texture_path);
    }

    BarDrawable bar_drawable(std::ref(this->_rectangle),
                             new_pos + offset,
                             sf::Vector2f(size.x, size.y),
                             bar.color,
                             fill_percentage,
                             bar.texture_path,
                             position.z,
                             true);

    all_drawables.emplace_back(DrawableVariant {std::move(bar_drawable)},
                               position.z);
  }
}

void SFMLRenderer::render_animated_sprites(
    Registry& r,
    std::vector<DrawableItem>& all_drawables,
    float min_dimension,
    const sf::Vector2u& window_size,
    const sf::Vector2f& view_size,
    const sf::Vector2f& view_pos)
{
  for (auto&& [entity, pos, draw, anim] :
       ZipperIndex<Position, Drawable, AnimatedSprite>(r))
  {
    if (!draw.enabled) {
      continue;
    }
    if (!anim.animations.contains(anim.current_animation)) {
      continue;
    }

    float offset_x = (window_size.x - min_dimension) / deux;
    float offset_y = (window_size.y - min_dimension) / deux;

    sf::Vector2f new_pos(
        static_cast<float>((pos.pos.x + 1.0) * min_dimension / deux) + offset_x,
        static_cast<float>((pos.pos.y + 1.0) * min_dimension / deux)
            + offset_y);

    AnimationData anim_data = anim.animations.at(anim.current_animation);

    if (new_pos.x + (anim_data.sprite_size.x * window_size.x)
            < view_pos.x - (view_size.x / 2)
        || new_pos.x - (anim_data.sprite_size.x * window_size.x)
            > view_pos.x + (view_size.x / 2))
    {
      continue;
    }
    if (new_pos.y + (anim_data.sprite_size.y * window_size.y)
            < view_pos.y - (view_size.y / 2)
        || new_pos.y - (anim_data.sprite_size.y * window_size.y)
            > view_pos.y + (view_size.y / 2))
    {
      continue;
    }

    sf::Texture& texture = load_texture(anim_data.texture_path);

    // this is to make the animated sprite take all the sheet if frame size is
    // like equal to -1
    if (anim_data.frame_size.x < 0) {
      anim_data.frame_size.x = texture.getSize().x;
    }
    if (anim_data.frame_size.y < 0) {
      anim_data.frame_size.y = texture.getSize().y;
    }

    float scale_x = static_cast<float>(min_dimension * anim_data.sprite_size.x)
        / anim_data.frame_size.x;
    float scale_y = static_cast<float>(min_dimension * anim_data.sprite_size.y)
        / anim_data.frame_size.y;
    float uniform_scale = std::min(scale_x, scale_y);

    float rotation = 0.0f;

    auto facings = this->_registry.get().get_components<Facing>();

    if (facings.size() > entity && facings.at(entity).has_value()) {
      if (facings.at(entity).value().plane) {
        Vector2D dir = facings[entity].value().direction.normalize();
        rotation = static_cast<float>(std::atan2(dir.y, dir.x) * 180.0 / M_PI);
      } else {
        Vector2D norm = (pos.pos - facings[entity].value().direction).normalize();
        rotation = static_cast<float>(std::atan2(norm.y, norm.x) * 180.0 / M_PI);
      }
    }

    if (!this->_sprite.has_value()) {
      this->_sprite = sf::Sprite(texture);
    }

    draw.true_size = Vector2D(
        std::max(static_cast<double>(anim_data.frame_size.x * uniform_scale)
                     / min_dimension,
                 draw.true_size.x),
        std::max(static_cast<double>(anim_data.frame_size.y * uniform_scale)
                     / min_dimension,
                 draw.true_size.y));

    AnimatedSpriteDrawable anim_drawable(
        std::ref(*this->_sprite),
        anim_data.texture_path,
        new_pos,
        sf::Vector2f(uniform_scale, uniform_scale),
        anim_data,
        rotation,
        pos.z);

    all_drawables.emplace_back(DrawableVariant {std::move(anim_drawable)},
                               pos.z);
  }
}

void SFMLRenderer::unified_render_system(Registry& r)
{
  std::vector<DrawableItem> all_drawables;
  sf::Vector2u window_size = _window.getSize();
  sf::Vector2f view_size = this->_view.getSize();
  sf::Vector2f view_pos = this->_view.getCenter();
  float min_dimension =
      static_cast<float>(std::min(window_size.x, window_size.y));

  render_sprites(
      r, all_drawables, min_dimension, window_size, view_size, view_pos);
  render_bars(r, all_drawables, min_dimension, window_size);
  render_animated_sprites(
      r, all_drawables, min_dimension, window_size, view_size, view_pos);
  render_sliders(r, all_drawables, min_dimension, window_size);
  render_texts(r, all_drawables, min_dimension, window_size);

  std::sort(all_drawables.begin(), all_drawables.end());

  for (auto& drawable : all_drawables) {
    drawable.draw(_window, this->_textures, this->_fonts);
  }
  this->_sprite->setRotation(sf::degrees(0));
}

void SFMLRenderer::hover_system(Registry& r)
{
  sf::Vector2i tmp = sf::Mouse::getPosition(_window);
  Vector2D mouse_pos = screen_to_world(tmp);

  for (const auto&& [e, clickable, pos, collision] :
       ZipperIndex<Clickable, Position, Collidable>(r))
  {
    if (!r.is_in_main_scene(e)) {
      continue;
    }
    Rect entity_rect = {.x = pos.pos.x,
                        .y = pos.pos.y,
                        .width = collision.size.x * 2,
                        .height = collision.size.y * 2};
    if (entity_rect.contains(mouse_pos.x, mouse_pos.y)) {
      if (r.has_component<Input>(e)) {
        this->_window.setMouseCursor(this->_cursors.at("text"));
      } else {
        this->_window.setMouseCursor(this->_cursors.at("hand"));
      }
      return;
    }
  }
  this->_window.setMouseCursor(this->_cursors.at("arrow"));
}

void SFMLRenderer::button_system(Registry& r)
{
  sf::Vector2i tmp = sf::Mouse::getPosition(_window);
  Vector2D mouse_pos = screen_to_world(tmp);

  for (auto&& [e, draw, anim, button, pos, collision] :
       ZipperIndex<Drawable, AnimatedSprite, Button, Position, Collidable>(r))
  {
    if (!draw.enabled || !r.is_in_main_scene(e)) {
      continue;
    }

    if (!anim.animations.contains("hover")
        || !anim.animations.contains("pressed")
        || !anim.animations.contains("idle"))
    {
      continue;
    }
    AnimationData hover_anim_data = anim.animations.at("hover");
    Rect entity_rect = {.x = pos.pos.x,
                        .y = pos.pos.y,
                        .width = collision.size.x * 2,
                        .height = collision.size.y * 2};
    if (entity_rect.contains(mouse_pos.x, mouse_pos.y)) {
      if (!button.hovered) {
        button.hovered = true;
        this->_event_manager.get().emit<PlayAnimationEvent>(
            "hover", e, hover_anim_data.framerate, true, false);
      }
    } else {
      if (button.hovered) {
        button.hovered = false;
        this->_event_manager.get().emit<PlayAnimationEvent>(
            "idle", e, hover_anim_data.framerate, true, false);
      }
    }
  }
}

extern "C"
{
PLUGIN_EXPORT void* entry_point(Registry& r, EventManager& em, EntityLoader& e)
{
  return new SFMLRenderer(r, em, e);
}
}
