
#include <functional>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <variant>

#include "SFMLRenderer.hpp"

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Event.hpp>

#include "Events.hpp"
#include "Json/JsonParser.hpp"
#include "ecs/Registery.hpp"
#include "ecs/zipper/Zipper.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"
#include "Vector2D.hpp"

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

SFMLRenderer::SFMLRenderer(Registery& r, EntityLoader& l)
    : APlugin(r,
              l,
              {"moving"},
              {COMP_INIT(Drawable, init_drawable),
               COMP_INIT(Sprite, init_sprite),
               COMP_INIT(Text, init_text)})
{
  _window = sf::RenderWindow(sf::VideoMode(window_size),
                                               "R-Type - SFML Renderer");
  _window.setFramerateLimit(window_rate);

  _registery.get().register_component<Drawable>();
  _registery.get().register_component<Sprite>();
  _registery.get().register_component<Text>();

  _registery.get().add_system<>([this](Registery&)
                                { _window.clear(sf::Color::Black); });
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

  _registery.get().add_system<>([this](Registery&) { this->handle_window(); });
  _textures.insert_or_assign(
      SFMLRenderer::placeholder_texture, gen_placeholder());
}

SFMLRenderer::~SFMLRenderer()
{
  if (_window.isOpen()) {
    _window.close();
  }
}

sf::Texture &SFMLRenderer::load_texture(std::string const& path)
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

sf::Font &SFMLRenderer::load_font(std::string const& path)
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

static Vector2D parse_vector2d(JsonVariant const& variant)
{
  try {
    JsonObject obj = std::get<JsonObject>(variant);
    double x = std::get<double>(obj.at("width").value);
    double y = std::get<double>(obj.at("height").value);
    return {x / SFMLRenderer::window_size.x, y / SFMLRenderer::window_size.y};
  } catch (std::bad_variant_access const&) {
    try {
      JsonObject obj = std::get<JsonObject>(variant);
      std::string x_percentage = std::get<std::string>(obj.at("width").value);
      std::string y_percentage = std::get<std::string>(obj.at("height").value);
      if (x_percentage.back() == '%') {
        x_percentage.pop_back();
      }
      if (y_percentage.back() == '%') {
        y_percentage.pop_back();
      }
      double x = std::stod(x_percentage) / 100.0;
      double y = std::stod(y_percentage) / 100.0;
      return {x, y};
    } catch (std::bad_variant_access const&) {
      return {10 / 100.0, 10 / 100.0};
    }
  }
}

void SFMLRenderer::init_sprite(Registery::Entity const entity,
                               JsonVariant const &config)
{
  try {
    JsonObject obj = std::get<JsonObject>(config);
    std::string texture_path = std::get<std::string>(obj.at("texture").value);
    Vector2D scale(0.1, 0.1);
    if (obj.contains("size")) {
      scale = parse_vector2d(obj.at("size").value); //la scale est en pourcentage de la taille de la window
    }
    _registery.get().emplace_component<Sprite>(entity, texture_path, scale);
  } catch (std::bad_variant_access const&) {
    LOGGER("SFML",
      LogLevel::ERROR,
      "Error loading sprite component: unexpected value type")
    } catch (std::out_of_range const &e) {
    LOGGER("SFML",
           LogLevel::ERROR,
           std::format("Error loading sprite component: missing value {} in JsonObject", e.what()))
  }
}

void SFMLRenderer::init_text(Registery::Entity const entity,
                             JsonVariant const &config)
{
  try {
    JsonObject obj = std::get<JsonObject>(config);
    std::string font_path;
    if (!obj.contains("font")) {
      throw std::out_of_range("font");
      return;
    }
    font_path = std::get<std::string>(obj.at("font").value);
    Vector2D scale(0.1, 0.1);
    if (!obj.contains("size")) {
      throw std::out_of_range("size");
      return;
    }
    scale = parse_vector2d(obj.at("size").value); //la scale est en pourcentage de la taille de la window
    std::string text;
    if (!obj.contains("text")) {
      throw std::out_of_range("text");
      return;
    }
    text = std::get<std::string>(obj.at("text").value);
    auto txt = _registery.get().emplace_component<Text>(entity, font_path, scale, text);
  } catch (std::bad_variant_access const&) {
    LOGGER("SFML",
           LogLevel::ERROR,
           "Error loading text component: unexpected value type")
  } catch (std::out_of_range const &e) {
    LOGGER("SFML",
           LogLevel::ERROR,
           std::format("Error loading text component: missing {} in JsonObject", e.what()))
  }
}

void SFMLRenderer::handle_resize() {
  sf::Vector2u new_size = _window.getSize();
  sf::View view(sf::Vector2f(static_cast<float>(new_size.x) / 2, static_cast<float>(new_size.y) / 2),
                sf::Vector2f(static_cast<float>(new_size.x),
                              static_cast<float>(new_size.y)));
  _window.setView(view);
}

void SFMLRenderer::handle_window()
{
  if (!_window.isOpen()) {
    return;
  }

  while (const std::optional event = _window.pollEvent()) {
    if (event->is<sf::Event::Closed>()) {
      _window.close();
      _registery.get().emit<ShutdownEvent>("Window closed", 0);
    }
    if (event->is<sf::Event::Resized>()) {
      handle_resize();
    }
  }

  _window.display();
}

void SFMLRenderer::render_sprites(Registery &/*unused*/,
                                  SparseArray<Position> positions,
                                  SparseArray<Drawable> drawable,
                                  SparseArray<Sprite> sprites)
{
  for (auto&& [pos, draw, spr] : Zipper(positions, drawable, sprites)) {
    sf::Texture &texture = load_texture(spr.texture_path);
    sf::Vector2u window_size = _window.getSize();
  
    if (!_sprite.has_value()) {
      _sprite.emplace(texture);
    }
    _sprite.value().setPosition(sf::Vector2f(static_cast<float>(pos.x), static_cast<float>(pos.y)));
    _sprite.value().setTexture(texture);
    _sprite.value().setScale(sf::Vector2f(
        static_cast<float>(window_size.x * spr.scale.x) / texture.getSize().x,
        static_cast<float>(window_size.x * spr.scale.y) / texture.getSize().y));
    _window.draw(_sprite.value());
  }
}

void SFMLRenderer::render_text(Registery &/*unused*/,
                               SparseArray<Position> positions,
                               SparseArray<Drawable> drawable,
                               SparseArray<Text> texts)
{
  for (auto&& [pos, draw, txt] : Zipper(positions, drawable, texts)) {
    sf::Font &font = load_font(txt.font_path);
    if (!_text.has_value()) {
      _text.emplace(font);
    }
    _text.value().setFont(font);
    _text.value().setString(txt.text);
    _text.value().setPosition(sf::Vector2f(static_cast<float>(pos.x), static_cast<float>(pos.y)));
    _text.value().setCharacterSize(static_cast<unsigned int>(window_size.x * txt.scale.x));
    _window.draw(_text.value());
  }
}

extern "C"
{
void* entry_point(Registery& r, EntityLoader& e)
{
  return new SFMLRenderer(r, e);
}
}
