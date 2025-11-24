#include <iostream>
#include <stdexcept>
#include <variant>

#include "SFMLRenderer.hpp"

#include "ecs/Registery.hpp"
#include "plugin/EntityLoader.hpp"

sfml_renderer::sfml_renderer(Registery& r, EntityLoader& l)
    : APlugin(
          r,
          l,
          {COMP_INIT(position, init_position), COMP_INIT(sprite, init_sprite)})
{
  window_ = std::make_unique<sf::RenderWindow>(
      sf::VideoMode(sf::Vector2u(1800, 1600)), "R-Type - SFML Renderer");
  window_->setFramerateLimit(60);

  registery_.get().registerComponent<position>();
  registery_.get().registerComponent<sprite>();

  registery_.get().addSystem<position, sprite>(
      [this](Registery& r, SparseArray<position> pos, SparseArray<sprite> spr)
      { this->render_system(r, pos, spr); });
}

sfml_renderer::~sfml_renderer()
{
  if (window_ && window_->isOpen()) {
    window_->close();
  }
}

void sfml_renderer::init_position(Registery::Entity const entity,
                                  JsonVariant const& config)
{
  try {
    JsonObject obj = std::get<JsonObject>(config);
    float x = static_cast<float>(std::get<double>(obj.at("x").value));
    float y = static_cast<float>(std::get<double>(obj.at("y").value));
    registery_.get().emplace_component<position>(entity, x, y);
  } catch (std::bad_variant_access const&) {
    std::cerr << "Error loading position component: unexpected value type"
              << '\n';
  } catch (std::out_of_range const&) {
    std::cerr << "Error loading position component: missing value in JsonObject"
              << '\n';
  }
}

void sfml_renderer::init_sprite(Registery::Entity const entity,
                                JsonVariant const& config)
{
  try {
    JsonObject obj = std::get<JsonObject>(config);
    std::string texture_path = std::get<std::string>(obj.at("texture").value);

    std::cout << "Loading sprite with texture: " << texture_path << '\n';

    auto spr = registery_.get().emplace_component<sprite>(entity, texture_path);
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

void sfml_renderer::render_system(Registery& r,
                                  SparseArray<position> positions,
                                  SparseArray<sprite> sprites)
{
  if (!window_ || !window_->isOpen()) {
    return;
  }

  while (const std::optional event = window_->pollEvent()) {
    if (event->is<sf::Event::Closed>()) {
      window_->close();
    }
  }

  window_->clear(sf::Color::Black);

  for (auto&& [pos, spr] : Zipper(positions, sprites)) {
    if (spr.sfml_sprite && spr.texture) {
      spr.sfml_sprite->setPosition(sf::Vector2f(pos.x, pos.y));
      window_->draw(*spr.sfml_sprite);
      std::cout << "Drawing sprite at (" << pos.x << ", " << pos.y << ")\n";
    } else {
      std::cout << "Sprite or texture is null\n";
    }
  }

  window_->display();
}

extern "C"
{
void* entry_point(Registery& r, EntityLoader& e)
{
  return new sfml_renderer(r, e);
}
}
