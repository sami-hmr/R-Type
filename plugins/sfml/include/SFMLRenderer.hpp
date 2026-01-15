#pragma once

#include <array>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <SFML/Audio/Music.hpp>
#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/PrimitiveType.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/VertexBuffer.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window.hpp>
#include <SFML/Window/Cursor.hpp>

#include "Drawable.hpp"
#include "Json/JsonParser.hpp"
#include "ecs/EventManager.hpp"
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
  SFMLRenderer(Registry& r, EventManager& em, EntityLoader& l);
  ~SFMLRenderer() override;

  static const int MAX_NB_SOUNDS = 16;
  static const int MAX_NB_MUSICS = 4;
  static constexpr sf::Vector2u window_size = {1080, 1080};
  static const std::size_t window_rate = 60;
  static constexpr sf::Vector2u placeholder_size = {50, 50};
  static constexpr std::string placeholder = "placeholder ";

private:
  sf::Texture& load_texture(std::string const& path);
  sf::Font& load_font(std::string const& path);
  sf::SoundBuffer& load_sound(std::string const& path);
  sf::Music& load_music(std::string const& path);
  std::optional<std::reference_wrapper<sf::Sound>> get_available_sound(sf::SoundBuffer& buffer);

  void handle_events();
  void mouse_events(const sf::Event& events);
  void handle_resize();
  Vector2D screen_to_world(sf::Vector2i screen_pos);
  void unified_render_system(Registry& r);
  void background_system(Registry& r);
  void camera_system(Registry& r);
  void button_system(Registry& r);
  void slider_system(Registry& r) const;
  void sounds_system(Registry& r);
  void musics_system(Registry& r);
  void hover_system(Registry &r);
  void display();
  
  void on_input_focus(const InputFocusEvent &/*unused*/);
  void on_click(const MousePressedEvent &/*unused*/);

  void render_sprites(Registry& r,
                      std::vector<DrawableItem>& all_drawables,
                      float min_dimension,
                      const sf::Vector2u& window_size,
                      const sf::Vector2f& view_size,
                      const sf::Vector2f& view_pos);
  void render_texts(Registry& r,
                    std::vector<DrawableItem>& all_drawables,
                    float min_dimension,
                    const sf::Vector2u& window_size);
  void render_bars(Registry& r,
                   std::vector<DrawableItem>& all_drawables,
                   float min_dimension,
                   const sf::Vector2u& window_size);
  void render_animated_sprites(Registry& r,
                               std::vector<DrawableItem>& all_drawables,
                               float min_dimension,
                               const sf::Vector2u& window_size,
                               const sf::Vector2f& view_size,
                               const sf::Vector2f& view_pos);
  void render_sliders(Registry& r,
                      std::vector<DrawableItem>& all_drawables,
                      float min_dimension,
                      const sf::Vector2u& window_size);

  std::optional<Key> sfml_key_to_key(sf::Keyboard::Key sfml_key);

  sf::RenderWindow _window;
  std::chrono::time_point<std::chrono::high_resolution_clock> _last_update;
  Vector2D _mouse_pos;

  std::unordered_map<std::string, sf::Texture> _textures;
  std::unordered_map<std::string, sf::Font> _fonts;

  std::optional<sf::Sprite> _sprite;
  std::optional<sf::Text> _text;
  sf::RectangleShape _rectangle;
  sf::CircleShape _circle;

  std::unordered_map<std::string, sf::SoundBuffer> _sound_buffers;
  std::array<std::optional<sf::Sound>, MAX_NB_SOUNDS> _sounds;
  std::map<std::string, sf::Music> _musics;

  sf::View _view;
  bool _camera_initialized = false;
  std::map<std::string, sf::Cursor> _cursors;


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

static const std::map<sf::Keyboard::Key, Key> key_association = {
    {sf::Keyboard::Key::Enter, Key::ENTER},
    {sf::Keyboard::Key::Left, Key::LEFT},
    {sf::Keyboard::Key::Right, Key::RIGHT},
    {sf::Keyboard::Key::Down, Key::DOWN},
    {sf::Keyboard::Key::Up, Key::UP},
    {sf::Keyboard::Key::Escape, Key::ECHAP},
    {sf::Keyboard::Key::Backspace, Key::DELETE},
    {sf::Keyboard::Key::Space, Key::SPACE},
    {sf::Keyboard::Key::LShift, Key::SHIFT},
    {sf::Keyboard::Key::RShift, Key::SHIFT},
    {sf::Keyboard::Key::LControl, Key::CTRL},
    {sf::Keyboard::Key::RControl, Key::CTRL},
    {sf::Keyboard::Key::LAlt, Key::ALT},
    {sf::Keyboard::Key::RAlt, Key::ALT},
    {sf::Keyboard::Key::A, Key::A},
    {sf::Keyboard::Key::B, Key::B},
    {sf::Keyboard::Key::D, Key::D},
    {sf::Keyboard::Key::C, Key::C},
    {sf::Keyboard::Key::E, Key::E},
    {sf::Keyboard::Key::F, Key::F},
    {sf::Keyboard::Key::G, Key::G},
    {sf::Keyboard::Key::H, Key::H},
    {sf::Keyboard::Key::I, Key::I},
    {sf::Keyboard::Key::J, Key::J},
    {sf::Keyboard::Key::K, Key::K},
    {sf::Keyboard::Key::L, Key::L},
    {sf::Keyboard::Key::M, Key::M},
    {sf::Keyboard::Key::N, Key::N},
    {sf::Keyboard::Key::O, Key::O},
    {sf::Keyboard::Key::P, Key::P},
    {sf::Keyboard::Key::Q, Key::Q},
    {sf::Keyboard::Key::R, Key::R},
    {sf::Keyboard::Key::S, Key::S},
    {sf::Keyboard::Key::T, Key::T},
    {sf::Keyboard::Key::U, Key::U},
    {sf::Keyboard::Key::V, Key::V},
    {sf::Keyboard::Key::W, Key::W},
    {sf::Keyboard::Key::X, Key::X},
    {sf::Keyboard::Key::Y, Key::Y},
    {sf::Keyboard::Key::Z, Key::Z},
    {sf::Keyboard::Key::Slash, Key::SLASH},
    {sf::Keyboard::Key::Num1, Key::ONE},
    {sf::Keyboard::Key::Num2, Key::TWO},
    {sf::Keyboard::Key::Num3, Key::THREE},
    {sf::Keyboard::Key::Num4, Key::FOUR},
    {sf::Keyboard::Key::Num5, Key::FIVE},
    {sf::Keyboard::Key::Num6, Key::SIX},
    {sf::Keyboard::Key::Num7, Key::SEVEN},
    {sf::Keyboard::Key::Num8, Key::EIGHT},
    {sf::Keyboard::Key::Num9, Key::NINE},
    {sf::Keyboard::Key::Num0, Key::ZERO}};

static const std::map<sf::Mouse::Button, MouseButton> MOUSEBUTTONMAP = {
    {sf::Mouse::Button::Left, MouseButton::MOUSELEFT},
    {sf::Mouse::Button::Right, MouseButton::MOUSERIGHT},
    {sf::Mouse::Button::Middle, MouseButton::MOUSEMIDDLE},
};
