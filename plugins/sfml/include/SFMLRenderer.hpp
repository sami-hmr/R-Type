#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>

#include "Json/JsonParser.hpp"
#include "ecs/Registery.hpp"
#include "ecs/SparseArray.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"

struct Position
{
  Position(float x, float y)
      : x(x)
      , y(y)
  {
  }

  float x;
  float y;
};

struct Sprite
{
  Sprite(std::string texture_path)
      : texture_path(std::move(texture_path))
  {
  }

  Sprite(Sprite const& other)

      = default;

  Sprite(Sprite&& other) noexcept
      : texture_path(std::move(other.texture_path))
      , texture(std::move(other.texture))
      , sfml_sprite(std::move(other.sfml_sprite))
  {
  }

  Sprite& operator=(Sprite const& other)
  {
    if (this != &other) {
      texture_path = other.texture_path;
      texture = other.texture;
      sfml_sprite = other.sfml_sprite;
    }
    return *this;
  }

  Sprite& operator=(Sprite&& other) noexcept
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

class SfmlRenderer : public APlugin
{
public:
  SfmlRenderer(Registery& r, EntityLoader& l);
  ~SfmlRenderer() override;

private:
  void init_position(Registery::Entity entity, JsonVariant const& config);
  void init_sprite(Registery::Entity entity, JsonVariant const& config);

  void render_system(Registery& r,
                     const SparseArray<Position>& positions,
                     const SparseArray<Sprite>& sprites);

  std::unique_ptr<sf::RenderWindow> _window;
  const std::vector<std::string> depends_on;
};
