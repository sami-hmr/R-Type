
#include <iostream>
#include <memory>
#include <stdexcept>
#include <variant>

#include "SFMLRenderer.hpp"

#include <SFML/Graphics/Sprite.hpp>

#include "Events.hpp"
#include "Json/JsonParser.hpp"
#include "ecs/Registery.hpp"
#include "ecs/zipper/Zipper.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"

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

SFMLRenderer::SFMLRenderer(Registery& r, EntityLoader& l)
    : APlugin(r,
              l,
              {"moving"},
              {COMP_INIT(Drawable, init_drawable),
               COMP_INIT(Sprite, init_sprite),
               COMP_INIT(Text, init_text)})
{
  _window = std::make_unique<sf::RenderWindow>(sf::VideoMode(window_size),
                                               "R-Type - SFML Renderer");
  _window->setFramerateLimit(window_rate);

  _registery.get().register_component<Drawable>();
  _registery.get().register_component<Sprite>();
  _registery.get().register_component<Text>();

  _registery.get().add_system<>(
      [this](Registery&) { this->handle_events(); }, 2);
  _registery.get().add_system<>(
      [this](Registery&) { _window->clear(sf::Color::Black); });
  _registery.get().add_system<Position, Drawable, Sprite>(
      [this](Registery& r,
             SparseArray<Position>& pos,
             SparseArray<Drawable>& draw,
             SparseArray<Sprite>& spr)
      { this->render_sprites(r, pos, draw, spr); });

  _registery.get().add_system<Position, Drawable, Text>(
      [this](Registery& r,
             const SparseArray<Position>& pos,
             const SparseArray<Drawable>& draw,
             const SparseArray<Text>& txt)
      { this->render_text(r, pos, draw, txt); });

  _registery.get().add_system<>([this](Registery&) { this->display(); });
}

SFMLRenderer::~SFMLRenderer()
{
  if (_window && _window->isOpen()) {
    _window->close();
  }
}

std::shared_ptr<sf::Texture> SFMLRenderer::load_texture(std::string const& path)
{
  auto it = _textures.find(path);
  if (it != _textures.end()) {
    return it->second;
  }

  auto texture = std::make_shared<sf::Texture>();
  if (!texture->loadFromFile(path)) {
    LOGGER("SFML", LogLevel::ERROR, "Failed to load texture: " + path)
    return nullptr;
  }
  _textures[path] = texture;
  return texture;
}

std::shared_ptr<sf::Font> SFMLRenderer::load_font(std::string const& path)
{
  auto it = _fonts.find(path);
  if (it != _fonts.end()) {
    return it->second;
  }

  auto font = std::make_shared<sf::Font>();
  if (!font->openFromFile(path)) {
    LOGGER("SFML", LogLevel::ERROR, "Failed to font texture: " + path)
    return nullptr;
  }
  _fonts[path] = font;
  return font;
}

void SFMLRenderer::init_drawable(Registery::Entity const entity,
                                 JsonVariant const& config)
{
  try {
    JsonObject obj = std::get<JsonObject>(config);
    _registery.get().emplace_component<Drawable>(entity);
  } catch (std::bad_variant_access const&) {
    LOGGER("SFML",
           LogLevel::ERROR,
           "Error loading sprite component: unexpected value type")
  } catch (std::out_of_range const&) {
    LOGGER("SFML",
           LogLevel::ERROR,
           "Error loading drawable component: missing value in JsonObject")
  }
}

void SFMLRenderer::init_sprite(Registery::Entity const entity,
                               JsonVariant const& config)
{
  try {
    JsonObject obj = std::get<JsonObject>(config);
    std::string texture_path = std::get<std::string>(obj.at("texture").value);
    _registery.get().emplace_component<Sprite>(entity, texture_path);

  } catch (std::bad_variant_access const&) {
    LOGGER("SFML",
           LogLevel::ERROR,
           "Error loading sprite component: unexpected value type")
  } catch (std::out_of_range const&) {
    LOGGER("SFML",
           LogLevel::ERROR,
           "Error loading sprite component: missing value in JsonObject")
  }
}

void SFMLRenderer::init_text(Registery::Entity const entity,
                             JsonVariant const& config)
{
  try {
    JsonObject obj = std::get<JsonObject>(config);
    std::string font_path = std::get<std::string>(obj.at("font").value);
    auto txt = _registery.get().emplace_component<Text>(entity, font_path);
  } catch (std::bad_variant_access const&) {
    LOGGER("SFML",
           LogLevel::ERROR,
           "Error loading text component: unexpected value type")
  } catch (std::out_of_range const&) {
    LOGGER("SFML",
           LogLevel::ERROR,
           "Error loading text component: missing value in JsonObject")
  }
}

std::optional<Key> SFMLRenderer::sfml_key_to_key(sf::Keyboard::Key sfml_key)
{
  auto it = key_association.find(sfml_key);
  if (it != key_association.end()) {
    return it->second;
  }
  return std::nullopt;
}

void SFMLRenderer::handle_events()
{
  _key_pressed.key_pressed.clear();
  if (_key_pressed.key_unicode.has_value()) {
    _key_pressed.key_unicode->clear();
  }

  _key_released.key_released.clear();
  if (_key_released.key_unicode.has_value()) {
    _key_released.key_unicode->clear();
  }

  while (const std::optional event = _window->pollEvent()) {
    if (event->is<sf::Event::Closed>()) {
      _window->close();
      _registery.get().emit<ShutdownEvent>("Window closed", 0);
    }
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
  }
  if (!_key_pressed.key_pressed.empty()) {
    _registery.get().emit<KeyPressedEvent>(_key_pressed);
  }
  if (!_key_released.key_released.empty()) {
    _registery.get().emit<KeyReleasedEvent>(_key_released);
  }
}

void SFMLRenderer::display()
{
  if (!_window || !_window->isOpen()) {
    return;
  }
  _window->display();
}

void SFMLRenderer::render_sprites(Registery& /*unused*/,
                                  const SparseArray<Position>& positions,
                                  const SparseArray<Drawable>& drawable,
                                  const SparseArray<Sprite>& sprites)
{
  for (auto&& [pos, draw, spr] : Zipper(positions, drawable, sprites)) {
    auto texture = load_texture(spr.texture_path);

    if (!texture) {
      continue;
    }
    if (!_sprite.has_value()) {
      _sprite.emplace(*texture);
    }
    _sprite->setPosition(sf::Vector2f(pos.x, pos.y));
    _sprite->setTexture(*texture);
    if (!_window || !_window->isOpen()) {
      continue;
    }
    _window->draw(*_sprite);
  }
}

void SFMLRenderer::render_text(Registery& /*unused*/,
                               const SparseArray<Position>& positions,
                               const SparseArray<Drawable>& drawable,
                               const SparseArray<Text>& texts)
{
  for (auto&& [pos, draw, txt] : Zipper(positions, drawable, texts)) {
    auto font = load_font(txt.font_path);
    if (!_text.has_value()) {
      _text.emplace(*font);
    }
    _text->setFont(*font);
    _text->setPosition(sf::Vector2f(pos.x, pos.y));
    if (!_window || !_window->isOpen()) {
      continue;
    }
    _window->draw(*_text);
  }
}

extern "C"
{
void* entry_point(Registery& r, EntityLoader& e)
{
  return new SFMLRenderer(r, e);
}
}
