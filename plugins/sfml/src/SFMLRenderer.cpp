
#include <algorithm>
#include <cstdio>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <tuple>

#include "SFMLRenderer.hpp"

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>

#include "../../collision/include/algorithm/Rect.hpp"
#include "ecs/Registry.hpp"
#include "ecs/Scenes.hpp"
#include "ecs/SparseArray.hpp"
#include "ecs/zipper/Zipper.hpp"
#include "ecs/zipper/ZipperIndex.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/components/AnimatedSprite.hpp"
#include "plugin/components/Background.hpp"
#include "plugin/components/Camera.hpp"
#include "plugin/components/Clickable.hpp"
#include "plugin/components/Collidable.hpp"
#include "plugin/components/Drawable.hpp"
#include "plugin/components/Position.hpp"
#include "plugin/components/Sprite.hpp"
#include "plugin/components/Text.hpp"
#include "plugin/events/AnimationEvents.hpp"
#include "plugin/events/DamageEvent.hpp"
#include "plugin/events/IoEvents.hpp"
#include "plugin/events/LoggerEvent.hpp"
#include "plugin/events/ShutdownEvent.hpp"

static const std::map<sf::Keyboard::Key, Key> key_association = {
    {sf::Keyboard::Key::Enter, Key::ENTER},
    {sf::Keyboard::Key::Left, Key::LEFT},
    {sf::Keyboard::Key::Right, Key::RIGHT},
    {sf::Keyboard::Key::Down, Key::DOWN},
    {sf::Keyboard::Key::Up, Key::UP},
    {sf::Keyboard::Key::Z, Key::Z},
    {sf::Keyboard::Key::Q, Key::Q},
    {sf::Keyboard::Key::S, Key::S},
    {sf::Keyboard::Key::D, Key::D},
    {sf::Keyboard::Key::R, Key::R},
    {sf::Keyboard::Key::Escape, Key::ECHAP},
    {sf::Keyboard::Key::Backspace, Key::DELETE},
    {sf::Keyboard::Key::Space, Key::SPACE},
    {sf::Keyboard::Key::LShift, Key::SHIFT},
    {sf::Keyboard::Key::RShift, Key::SHIFT},
    {sf::Keyboard::Key::LControl, Key::CTRL},
    {sf::Keyboard::Key::RControl, Key::CTRL},
    {sf::Keyboard::Key::LAlt, Key::ALT},
    {sf::Keyboard::Key::RAlt, Key::ALT},
};

static const std::map<sf::Mouse::Button, MouseButton> MOUSEBUTTONMAP = {
    {sf::Mouse::Button::Left, MouseButton::MOUSELEFT},
    {sf::Mouse::Button::Right, MouseButton::MOUSERIGHT},
    {sf::Mouse::Button::Middle, MouseButton::MOUSEMIDDLE},
};

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

SFMLRenderer::SFMLRenderer(Registry& r, EntityLoader& l)
    : APlugin("sfml",
              r,
              l,
              {"moving", "ath", "ui", "client_network", "server_network"},
              {})
{
  _window =
      sf::RenderWindow(sf::VideoMode(window_size), "R-Type - SFML Renderer");
  _window.setFramerateLimit(window_rate);

  _registry.get().add_system([this](Registry&) { this->handle_events(); }, 1);
  _registry.get().add_system([this](Registry&)
                             { _window.clear(sf::Color::Black); });
  _registry.get().add_system([this](Registry& r)
                             { this->background_system(r); });

  _registry.get().add_system([this](Registry& r) { this->render_sprites(r); });

  _registry.get().add_system([this](Registry& r) { this->render_text(r); });

  _registry.get().add_system([this](Registry& r)
                             { this->animation_system(r); });
  _registry.get().add_system([this](Registry& r) { this->bar_system(r); });
  _registry.get().add_system([this](Registry& r) { this->camera_system(r); });
  _registry.get().add_system<>([this](Registry&) { this->display(); });
  _textures.insert_or_assign(SFMLRenderer::placeholder_texture,
                             gen_placeholder());

  SUBSCRIBE_EVENT(PlayAnimationEvent, {
    AnimatedSprite::on_play_animation(this->_registry.get(), event);
  })
  SUBSCRIBE_EVENT(AnimationEndEvent, {
    AnimatedSprite::on_animation_end(this->_registry.get(), event);
  })
  SUBSCRIBE_EVENT(DamageEvent,
                  { AnimatedSprite::on_death(this->_registry.get(), event); })
  SUBSCRIBE_EVENT(MousePressedEvent,
                  { this->on_click(this->_registry.get(), event); })
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
    LOGGER("SFML", LogLevel::ERROR, "Failed to load texture: " + path)
    return _textures.at(placeholder_texture);
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
    LOGGER("SFML", LogLevel::ERROR, "Failed to load font: " + path)
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

void SFMLRenderer::mouse_events(const sf::Event& events)
{
  sf::Vector2u window_size = _window.getSize();
  double min_dimension =
      static_cast<double>(std::min(window_size.x, window_size.y));
  const sf::Vector2i mouse_pos = sf::Mouse::getPosition(_window);
  const auto* mouse_pressed = events.getIf<sf::Event::MouseButtonPressed>();
  const auto* mouse_released = events.getIf<sf::Event::MouseButtonReleased>();

  if (mouse_pressed != nullptr) {
    if (MOUSEBUTTONMAP.contains(mouse_pressed->button)) {
      MouseButton button = MOUSEBUTTONMAP.at(mouse_pressed->button);
      Vector2D position((mouse_pos.x * deux / min_dimension) - 1.0,
                        (mouse_pos.y * deux / min_dimension) - 1.0);
      MousePressedEvent mouse_event(position, button);
      this->_registry.get().emit<MousePressedEvent>(mouse_event);
    }
  }
  if (mouse_released != nullptr) {
    if (MOUSEBUTTONMAP.contains(mouse_released->button)) {
      MouseButton button = MOUSEBUTTONMAP.at(mouse_released->button);
      Vector2D position((mouse_pos.x * deux / min_dimension) - 1.0,
                        (mouse_pos.y * deux / min_dimension) - 1.0);
      MouseReleasedEvent mouse_event(position, button);
      this->_registry.get().emit<MouseReleasedEvent>(mouse_event);
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
      _registry.get().emit<ShutdownEvent>("Window closed", 0);
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
      if (text_entered->unicode >= 'A' && text_entered->unicode < 'z') {
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
    _registry.get().emit<KeyPressedEvent>(_key_pressed);
  }
  if (!_key_released.key_released.empty()) {
    _registry.get().emit<KeyReleasedEvent>(_key_released);
  }
}

void SFMLRenderer::display()
{
  if (!_window.isOpen()) {
    return;
  }
  _window.display();
}

void SFMLRenderer::render_sprites(Registry& r)
{
  std::vector<
      std::
          tuple<std::reference_wrapper<sf::Texture>, double, sf::Vector2f, int>>
      drawables;
  sf::Vector2u window_size = _window.getSize();
  sf::Vector2f view_size = this->_view.getSize();
  sf::Vector2f view_pos = this->_view.getCenter();

  float min_dimension =
      static_cast<float>(std::min(window_size.x, window_size.y));

  drawables.reserve(std::max({r.get_components<Position>().size(),
                              r.get_components<Drawable>().size(),
                              r.get_components<Sprite>().size()}));

  for (auto&& [pos, draw, spr] : Zipper<Position, Drawable, Sprite>(r)) {
    if (!draw.enabled) {
      continue;
    }
    sf::Vector2f new_pos(
        static_cast<float>((pos.pos.x + 1.0) * min_dimension / 2.0f),
        static_cast<float>((pos.pos.y + 1.0) * min_dimension / 2.0f));

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
        static_cast<float>(window_size.x * spr.scale.x) / texture.getSize().x;
    float scale_y =
        static_cast<float>(window_size.y * spr.scale.y) / texture.getSize().y;
    float uniform_scale = std::min(scale_x, scale_y);

    drawables.emplace_back(std::ref(texture), uniform_scale, new_pos, pos.z);
  }
  std::sort(drawables.begin(),
            drawables.end(),
            [](auto const& a, auto const& b)
            { return std::get<3>(a) < std::get<3>(b); });

  for (auto&& [texture, scale, new_pos, z] : drawables) {
    if (!this->_sprite.has_value()) {
      this->_sprite.emplace(texture.get());
    } else {
      this->_sprite->setTexture(texture.get(), true);
    }
    this->_sprite->setOrigin(
        sf::Vector2f(static_cast<float>(texture.get().getSize().x) / 2.0f,
                     static_cast<float>(texture.get().getSize().y) / 2.0f));
    this->_sprite->setScale(sf::Vector2f(scale, scale));
    this->_sprite->setPosition(new_pos);
    _window.draw(*this->_sprite);
  }
}

void SFMLRenderer::render_text(Registry& r)
{
  for (auto&& [i, pos, draw, txt] : ZipperIndex<Position, Drawable, Text>(r)) {
    if (!draw.enabled) {
      continue;
    }

    sf::Font& font = load_font(txt.font_path);
    if (!_text.has_value()) {
      _text.emplace(font);
    }
    _text.value().setFont(font);

    _text.value().setString(txt.text);

    sf::Vector2u window_size = _window.getSize();
    float min_dimension =
        static_cast<float>(std::min(window_size.x, window_size.y));
    sf::Vector2f new_pos(
        static_cast<float>((pos.pos.x + 1.0) * min_dimension / 2.0f),
        static_cast<float>((pos.pos.y + 1.0) * min_dimension / 2.0f));
    _text.value().setPosition(new_pos);
    _text.value().setCharacterSize(static_cast<unsigned int>(txt.scale.x));
    _window.draw(_text.value());
  }
}

void SFMLRenderer::bar_system(Registry& r)
{
  sf::Vector2u window_size = _window.getSize();

  for (auto&& [scene, drawable, position, bar] :
       Zipper<Scene, Drawable, Position, Bar>(r))
  {
    this->_rectangle.setOutlineColor(sf::Color::Transparent);
    this->_rectangle.setFillColor(sf::Color::Transparent);
    if (!drawable.enabled) {
      continue;
    }
    float min_dimension =
        static_cast<float>(std::min(window_size.x, window_size.y));
    sf::Vector2f new_pos(
        static_cast<float>((position.pos.x + 1.0) * min_dimension / 2.0f),
        static_cast<float>((position.pos.y + 1.0) * min_dimension / 2.0f));
    sf::Vector2f size(static_cast<float>(bar.size.x * min_dimension),
                      static_cast<float>(bar.size.y * min_dimension));
    sf::Vector2f offset(static_cast<float>(bar.offset.x * min_dimension),
                        static_cast<float>(bar.offset.y * min_dimension));

    _rectangle.setPosition(new_pos + offset);
    _rectangle.setSize(size);
    _rectangle.setOrigin(sf::Vector2f(size.x / 2, size.y / 2));
    if (bar.outline) {
      _rectangle.setOutlineColor(
          sf::Color(bar.color.r, bar.color.g, bar.color.b, bar.color.a));
      _rectangle.setOutlineThickness(size.y * 0.1f);
      _window.draw(_rectangle);
    }

    float fill_percentage = bar.current_value / bar.max_value;
    if (fill_percentage < 0.0f) {
      fill_percentage = 0.0f;
    } else if (fill_percentage > 1.0f) {
      fill_percentage = 1.0f;
    }
    if (bar.texture_path != "") {
      sf::Texture& texture = load_texture(bar.texture_path);
      this->_rectangle.setTexture(&texture, true);
      this->_rectangle.setTextureRect(
          sf::IntRect({0, 0},
                      {static_cast<int>(texture.getSize().x * fill_percentage),
                       static_cast<int>(texture.getSize().y)}));
    }
    this->_rectangle.setSize(sf::Vector2f(size.x * fill_percentage, size.y));
    this->_rectangle.setOutlineColor(sf::Color::Transparent);
    this->_rectangle.setFillColor(
        sf::Color(bar.color.r, bar.color.g, bar.color.b, bar.color.a));
    this->_rectangle.setPosition(new_pos + offset);
    this->_window.draw(_rectangle);
  }
}

void SFMLRenderer::on_click(Registry& r, const MousePressedEvent& event)
{
  for (const auto& [draw, clickable, pos, collision] :
       Zipper<Drawable, Clickable, Position, Collidable>(r))
  {
    if (!draw.enabled) {
      continue;
    }
    Rect entity_rect = {.x = pos.pos.x,
                        .y = pos.pos.y,
                        .width = collision.width,
                        .height = collision.height};
    if (entity_rect.contains(event.position.x, event.position.y)) {
      for (const auto& [name, obj] : clickable.to_emit) {
        r.emit(name, obj);
        return;
      }
    }
  }
}

extern "C"
{
void* entry_point(Registry& r, EntityLoader& e)
{
  return new SFMLRenderer(r, e);
}
}
