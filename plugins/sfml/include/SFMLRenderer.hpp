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
#include <SFML/System/Vector2.hpp>
#include <SFML/Window.hpp>

#include "Json/JsonParser.hpp"
#include "ecs/Registery.hpp"
#include "ecs/SparseArray.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/components/Drawable.hpp"
#include "plugin/components/Position.hpp"
#include "plugin/components/Sprite.hpp"
#include "plugin/components/Text.hpp"
#include "plugin/events/Events.hpp"

class SFMLRenderer : public APlugin
{
public:
  SFMLRenderer(Registery& r, EntityLoader& l);
  ~SFMLRenderer() override;

  static constexpr sf::Vector2u window_size = {1080, 1080};
  static const std::size_t window_rate = 60;
  static constexpr sf::Vector2u placeholder_size = {50, 50};
  static constexpr std::string placeholder_texture = "placeholder ";

private:
  sf::Texture& load_texture(std::string const& path);
  sf::Font& load_font(std::string const& path);

  void init_drawable(Registery::Entity const& entity, JsonObject const& obj);
  void init_sprite(Registery::Entity const& entity, JsonObject const& obj);
  void init_text(Registery::Entity const& entity, JsonObject const& obj);

  template<typename T>
  Vector2D parse_vector2d(Registery::Entity const& entity,
                          JsonObject const& obj,
                          std::string const& str);

  void handle_events();
  void handle_resize();
  void render_sprites(Registery& r,
                      const SparseArray<Scene>& scenes,
                      const SparseArray<Position>& positions,
                      const SparseArray<Drawable>& drawable,
                      const SparseArray<Sprite>& sprites);
  void render_text(Registery& r,
                   const SparseArray<Scene>& scenes,
                   const SparseArray<Position>& positions,
                   const SparseArray<Drawable>& drawable,
                   const SparseArray<Text>& texts);
  void display();

  std::optional<Key> sfml_key_to_key(sf::Keyboard::Key sfml_key);

  sf::RenderWindow _window;
  std::chrono::time_point<std::chrono::high_resolution_clock> _last_update;

  std::unordered_map<std::string, sf::Texture> _textures;
  std::unordered_map<std::string, sf::Font> _fonts;

  std::optional<sf::Sprite> _sprite;
  std::optional<sf::Text> _text;

  KeyPressedEvent _key_pressed;
  KeyReleasedEvent _key_released;
};
