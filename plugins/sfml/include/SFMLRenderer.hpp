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
#include <SFML/System/Clock.hpp>
#include <SFML/Window.hpp>

#include "Json/JsonParser.hpp"
#include "ecs/Registery.hpp"
#include "ecs/SparseArray.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/components/Drawable.hpp"
#include "plugin/components/Position.hpp"
#include "plugin/components/Sprite.hpp"
#include "plugin/components/Text.hpp"

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

  void handle_events();
  void render_sprites(Registery& r,
                      SparseArray<Position> positions,
                      SparseArray<Drawable> drawable,
                      SparseArray<Sprite> sprites);
  void render_text(Registery& r,
                   SparseArray<Position> positions,
                   SparseArray<Drawable> drawable,
                   SparseArray<Text> texts);

  std::unique_ptr<sf::RenderWindow> _window;
  std::chrono::time_point<std::chrono::high_resolution_clock> _last_update;

  std::unordered_map<std::string, std::shared_ptr<sf::Texture>> _textures;
  std::unordered_map<std::string, std::shared_ptr<sf::Font>> _fonts;

  std::optional<sf::Sprite> _sprite;
  std::optional<sf::Text> _text;

  const std::vector<std::string> depends_on = {"moving"};
};
