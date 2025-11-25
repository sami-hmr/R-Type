
#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <variant>

#include "SFMLRenderer.hpp"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Window/VideoMode.hpp>

#include "Json/JsonParser.hpp"
#include "ecs/Registery.hpp"
#include "ecs/SparseArray.hpp"
#include "ecs/zipper/Zipper.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"

SfmlRenderer::SfmlRenderer(Registery& r, EntityLoader& l)
    : APlugin(
          r,
          l,
          {COMP_INIT(position, init_position), COMP_INIT(sprite, init_sprite)})
{
  _window = std::make_unique<sf::RenderWindow>(
      sf::VideoMode(sf::Vector2u(1800, 1600)), "R-Type - SFML Renderer");
  _window->setFramerateLimit(60);

  _registery.get().register_component<Position>();
  _registery.get().register_component<Sprite>();

  _registery.get().add_system<Position, Sprite>(
      [this](Registery& r,
             const SparseArray<Position>& pos,
             const SparseArray<Sprite>& spr)
      { this->render_system(r, pos, spr); });
}

SfmlRenderer::~SfmlRenderer()
{
  if (_window && _window->isOpen()) {
    _window->close();
  }
}

void SfmlRenderer::init_position(Registery::Entity const entity,
                                 JsonVariant const& config)
{
  try {
    JsonObject obj = std::get<JsonObject>(config);
    auto x = static_cast<float>(std::get<double>(obj.at("x").value));
    auto y = static_cast<float>(std::get<double>(obj.at("y").value));
    this->_registery.get().emplace_component<Position>(entity, x, y);
  } catch (std::bad_variant_access const&) {
    std::cerr << "Error loading position component: unexpected value type"
              << '\n';
  } catch (std::out_of_range const&) {
    std::cerr << "Error loading position component: missing value in JsonObject"
              << '\n';
  }
}

void SfmlRenderer::init_sprite(Registery::Entity const entity,
                               JsonVariant const& config)
{
  try {
    JsonObject obj = std::get<JsonObject>(config);
    std::string const texture_path =
        std::get<std::string>(obj.at("texture").value);

    std::cout << "Loading sprite with texture: " << texture_path << '\n';

    auto spr = _registery.get().emplace_component<Sprite>(entity, texture_path);
    if (spr) {
      spr->texture = std::make_shared<sf::Texture>();
      if (!spr->texture->loadFromFile(texture_path)) {
        std::cerr << "Failed to load texture: " << texture_path << '\n';
        return;
      }
      std::cout << "Texture loaded successfully: " << texture_path << '\n';
      spr->sfml_sprite = std::make_shared<sf::Sprite>(*spr->texture);
      std::cout << "Sprite created successfully\n";
    }
  } catch (std::bad_variant_access const&) {
    std::cerr << "Error loading sprite component: unexpected value type"
              << '\n';
  } catch (std::out_of_range const&) {
    std::cerr << "Error loading sprite component: missing value in JsonObject"
              << '\n';
  }
}

void SfmlRenderer::render_system(Registery& /*r*/,
                                 const SparseArray<Position>& positions,
                                 const SparseArray<Sprite>& sprites)
{
  if (!_window || !_window->isOpen()) {
    return;
  }

  while (const std::optional event = _window->pollEvent()) {
    if (event->is<sf::Event::Closed>()) {
      _window->close();
    }
  }

  _window->clear(sf::Color::Black);

  for (auto&& [pos, spr] : Zipper(positions, sprites)) {
    if (spr.sfml_sprite && spr.texture) {
      spr.sfml_sprite->setPosition(sf::Vector2f(pos.x, pos.y));
      _window->draw(*spr.sfml_sprite);
      std::cout << "Drawing sprite at (" << pos.x << ", " << pos.y << ")\n";
    } else {
      std::cout << "Sprite or texture is null\n";
    }
  }

  _window->display();
}

extern "C"
{
void* entry_point(Registery& r, EntityLoader& e)
{
  return new SfmlRenderer(r, e);
}
}
