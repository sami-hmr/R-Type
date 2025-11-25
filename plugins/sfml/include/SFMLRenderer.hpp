#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Window.hpp>

#include "Json/JsonParser.hpp"
#include "ecs/Registery.hpp"
#include "ecs/SparseArray.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/components/Position.hpp"

struct Drawable
{
  Drawable() = default;
};

struct Sprite
{
  Sprite(std::string texture_path)
      : texture_path(std::move(texture_path))
  {
  }

  std::string texture_path;
};

struct Text
{
  std::string font_path;
};

class SFMLRenderer : public APlugin
{
public:
  SFMLRenderer(Registery& r, EntityLoader& l);
  ~SFMLRenderer() override;

  static constexpr sf::Vector2u window_size = {1800, 1600};
  static constexpr std::size_t window_rate = 60;

private:
  std::shared_ptr<sf::Texture> load_texture(std::string const& path);
  std::shared_ptr<sf::Font> load_font(std::string const& path);

  void init_drawable(Registery::Entity const entity, JsonVariant const& config);
  void init_sprite(Registery::Entity const entity, JsonVariant const& config);
  void init_text(Registery::Entity const entity, JsonVariant const& config);

  void handle_window();
  void render_sprites(Registery& r,
                      SparseArray<Position> positions,
                      SparseArray<Drawable> drawable,
                      SparseArray<Sprite> sprites);
  void render_text(Registery& r,
                   SparseArray<Position> positions,
                   SparseArray<Drawable> drawable,
                   SparseArray<Text> texts);

  std::unique_ptr<sf::RenderWindow> _window;
  std::unordered_map<std::string, std::shared_ptr<sf::Texture>> _textures;
  std::unordered_map<std::string, std::shared_ptr<sf::Font>> _fonts;
  std::optional<sf::Sprite> _sprite;
  const std::vector<std::string> depends_on = {"Moving"};
};
