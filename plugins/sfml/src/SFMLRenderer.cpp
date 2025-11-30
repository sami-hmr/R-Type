
#include <memory>
#include <stdexcept>
#include <variant>

#include "SFMLRenderer.hpp"

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>

#include "ClientConnection.hpp"
#include "Events.hpp"
#include "Json/JsonParser.hpp"
#include "ServerLaunch.hpp"
#include "ecs/Registery.hpp"
#include "ecs/zipper/Zipper.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"

SFMLRenderer::SFMLRenderer(Registery& r, EntityLoader& l)
    : APlugin(r,
              l,
              {"moving", "client_network", "server_network"},
              {COMP_INIT(Drawable, init_drawable),
               COMP_INIT(Sprite, init_sprite),
               COMP_INIT(Text, init_text)})
{
  _window = std::make_unique<sf::RenderWindow>(sf::VideoMode(window_size),
                                               "R-Type - SFML Renderer");
  _window->setFramerateLimit(window_rate);

  _registery.get().register_component<Drawable>("sfml:Drawable");
  _registery.get().register_component<Sprite>("sfml:Sprite");
  _registery.get().register_component<Text>("sfml:Text");

  _registery.get().add_system<>([this](Registery&)
                                { _window->clear(sf::Color::Black); });
  _registery.get().add_system<Position, Drawable, Sprite>(
      [this](Registery& r,
             SparseArray<Position> pos,
             SparseArray<Drawable> draw,
             SparseArray<Sprite> spr)
      { this->render_sprites(r, pos, draw, spr); });

  _registery.get().add_system<Position, Drawable, Text>(
      [this](Registery& r,
             SparseArray<Position> pos,
             SparseArray<Drawable> draw,
             SparseArray<Text> txt) { this->render_text(r, pos, draw, txt); });

  _registery.get().add_system<>([this](Registery&) {
      this->handle_events();
      this->_window->display(); });
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

void SFMLRenderer::handle_events() {
    this->_window->handleEvents(
        [this](sf::Event::Closed const&)
        {
          this->_window->close();
          this->_registery.get().emit<ShutdownEvent>("Window closed", 0);
        },
        [this](sf::Event::KeyPressed const &key) {
            if (key.code == sf::Keyboard::Key::Space) {
                this->_registery.get().emit<ClientConnection>("127.0.0.1", 4242);
            }
            if (key.code == sf::Keyboard::Key::S) {
                this->_registery.get().emit<ServerLaunching>(4242);
            }
        });
}

void SFMLRenderer::render_sprites(Registery& /*unused*/,
                                  SparseArray<Position> positions,
                                  SparseArray<Drawable> drawable,
                                  SparseArray<Sprite> sprites)
{
  for (auto&& [pos, draw, spr] : Zipper(positions, drawable, sprites)) {
    auto texture = load_texture(spr.texture_path);

    if (!_sprite.has_value()) {
      _sprite.emplace(*texture);
    }
    _sprite->setPosition(sf::Vector2f(pos.x, pos.y));
    _sprite->setTexture(*texture);
    _window->draw(*_sprite);
  }
}

void SFMLRenderer::render_text(Registery& /*unused*/,
                               SparseArray<Position> positions,
                               SparseArray<Drawable> drawable,
                               SparseArray<Text> texts)
{
  for (auto&& [pos, draw, txt] : Zipper(positions, drawable, texts)) {
    auto font = load_font(txt.font_path);
    if (!_text.has_value()) {
      _text.emplace(*font);
    }
    _text->setFont(*font);
    _text->setPosition(sf::Vector2f(pos.x, pos.y));
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
