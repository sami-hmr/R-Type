#pragma once

#include <iostream>
#include <memory>

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

#include "Json/JsonParser.hpp"
#include "ecs/Registery.hpp"
#include "ecs/SparseArray.hpp"
#include "ecs/zipper/Zipper.hpp"
#include "plugin/APlugin.hpp"

struct position
{
  position(float x, float y)
      : x(x)
      , y(y)
  {
  }

  float x;
  float y;
};

struct sprite
{
  sprite(std::string const& texture_path)
      : texture_path(texture_path)
  {
  }

  sprite(sprite const& other)
      : texture_path(other.texture_path)
      , texture(other.texture)
      , sfml_sprite(other.sfml_sprite)
  {
  }

  sprite(sprite&& other) noexcept
      : texture_path(std::move(other.texture_path))
      , texture(std::move(other.texture))
      , sfml_sprite(std::move(other.sfml_sprite))
  {
  }

  sprite& operator=(sprite const& other)
  {
    if (this != &other) {
      texture_path = other.texture_path;
      texture = other.texture;
      sfml_sprite = other.sfml_sprite;
    }
    return *this;
  }

  sprite& operator=(sprite&& other) noexcept
  {
    if (this != &other) {
      texture_path = std::move(other.texture_path);
      texture = std::move(other.texture);
      sfml_sprite = std::move(other.sfml_sprite);
    }
    return *this;
  }

  std::string texture_path;
  std::shared_ptr<sf::Texture> texture;
  std::shared_ptr<sf::Sprite> sfml_sprite;
};

class sfml_renderer : public APlugin
{
public:
  sfml_renderer(Registery& r, EntityLoader& l);
  ~sfml_renderer() override;

private:
  void init_position(Registery::Entity const entity, JsonVariant const& config);
  void init_sprite(Registery::Entity const entity, JsonVariant const& config);

  void render_system(Registery& r,
                     SparseArray<position> positions,
                     SparseArray<sprite> sprites);

  std::unique_ptr<sf::RenderWindow> window_;
  const std::vector<std::string> depends_on_ = {};
};
