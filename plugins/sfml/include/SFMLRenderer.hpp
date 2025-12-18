#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window.hpp>

#include "Json/JsonParser.hpp"
#include "ecs/Registry.hpp"
#include "ecs/SparseArray.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/components/AnimatedSprite.hpp"
#include "plugin/components/Background.hpp"
#include "plugin/components/Bar.hpp"
#include "plugin/components/Camera.hpp"
#include "plugin/components/Drawable.hpp"
#include "plugin/components/Position.hpp"
#include "plugin/components/Sprite.hpp"
#include "plugin/components/Text.hpp"
#include "plugin/events/CameraEvents.hpp"
#include "plugin/events/IoEvents.hpp"

class SFMLRenderer : public APlugin
{
public:
  SFMLRenderer(Registry& r, EntityLoader& l);
  ~SFMLRenderer() override;

  static constexpr sf::Vector2u window_size = {1080, 1080};
  static const std::size_t window_rate = 60;
  static constexpr sf::Vector2u placeholder_size = {50, 50};
  static constexpr std::string placeholder_texture = "placeholder ";

private:
  sf::Texture& load_texture(std::string const& path);
  sf::Font& load_font(std::string const& path);

  void handle_events();
  void mouse_events(const sf::Event &events);
  void handle_resize();
  Vector2D screen_to_world(sf::Vector2i screen_pos);
  void render_sprites(Registry& r);
  void render_text(Registry& r);
  void background_system(Registry& r);
  void camera_system(Registry &r);
  void bar_system(Registry &r);
  void button_system(Registry& r);

  
  void animation_system(Registry& r);
  void display();

  std::optional<Key> sfml_key_to_key(sf::Keyboard::Key sfml_key);

  sf::RenderWindow _window;
  std::chrono::time_point<std::chrono::high_resolution_clock> _last_update;

  std::unordered_map<std::string, sf::Texture> _textures;
  std::unordered_map<std::string, sf::Font> _fonts;

  std::optional<sf::Sprite> _sprite;
  std::optional<sf::Text> _text;
  sf::RectangleShape _rectangle;

  sf::View _view;
  bool _camera_initialized = false;

  void draw_nothing_background(Background& background);
  void draw_repeat_background(Background& background);
  void draw_stretch_background(Background& background);

  std::map<Background::RenderType, std::function<void(Background&)>>
      _draw_functions {
          {Background::RenderType::NOTHING,
           [this](Background& background)
           { this->draw_nothing_background(background); }},
          {Background::RenderType::REPEAT,
           [this](Background& background)
           { this->draw_repeat_background(background); }},
          {Background::RenderType::STRETCH,
           [this](Background& background)
           { this->draw_stretch_background(background); }},
      };

  KeyPressedEvent _key_pressed;
  KeyReleasedEvent _key_released;
};
