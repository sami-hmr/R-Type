
#include <algorithm>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <tuple>

#include "SFMLRenderer.hpp"

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>

#include "Json/JsonParser.hpp"
#include "ecs/Registery.hpp"
#include "ecs/Scenes.hpp"
#include "ecs/SparseArray.hpp"
#include "ecs/zipper/Zipper.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/components/AnimatedSprite.hpp"
#include "plugin/components/Background.hpp"
#include "plugin/components/Drawable.hpp"
#include "plugin/components/Sprite.hpp"
#include "plugin/components/Text.hpp"
#include "plugin/events/Events.hpp"

static const std::map<sf::Keyboard::Key, Key> key_association = {
    {sf::Keyboard::Key::Enter, Key::ENTER},
    {sf::Keyboard::Key::Left, Key::LEFT},
    {sf::Keyboard::Key::Right, Key::RIGHT},
    {sf::Keyboard::Key::Down, Key::DOWN},
    {sf::Keyboard::Key::Up, Key::UP},
    {sf::Keyboard::Key::Z, Key::Z},
    {sf::Keyboard::Key::Q, Key::Q},
    {sf::Keyboard::Key::S, Key::S},
    {sf::Keyboard::Key::D, Key::D},
    {sf::Keyboard::Key::R, Key::R},
    {sf::Keyboard::Key::Escape, Key::ECHAP},
    {sf::Keyboard::Key::Backspace, Key::DELETE},
    {sf::Keyboard::Key::Space, Key::SPACE},
    {sf::Keyboard::Key::LShift, Key::SHIFT},
    {sf::Keyboard::Key::RShift, Key::SHIFT},
    {sf::Keyboard::Key::LControl, Key::CTRL},
    {sf::Keyboard::Key::RControl, Key::CTRL},
    {sf::Keyboard::Key::LAlt, Key::ALT},
    {sf::Keyboard::Key::RAlt, Key::ALT},
};

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
              {"moving", "client_network", "server_network"},
              {COMP_INIT(Drawable, Drawable, init_drawable),
               COMP_INIT(Sprite, Sprite, init_sprite),
               COMP_INIT(Text, Text, init_text),
               COMP_INIT(Background, Background, init_background),
               COMP_INIT(AnimatedSprite, AnimatedSprite, init_animated_sprite)})
{
  _window =
      sf::RenderWindow(sf::VideoMode(window_size), "R-Type - SFML Renderer");
  _window.setFramerateLimit(window_rate);

  _registery.get().register_component<Drawable>("sfml:Drawable");
  _registery.get().register_component<Sprite>("sfml:Sprite");
  _registery.get().register_component<Text>("sfml:Text");
  _registery.get().register_component<Background>("sfml:Background");
  _registery.get().register_component<AnimatedSprite>("sfml:AnimatedSprite");

  _registery.get().add_system<>([this](Registery&) { this->handle_events(); },
                                1);
  _registery.get().add_system<>([this](Registery&)
                                { _window.clear(sf::Color::Black); });
  _registery.get().add_system<Scene, Drawable, Background>(
      [this](Registery& r,
             const SparseArray<Scene>& scenes,
             const SparseArray<Drawable>& drawables,
             SparseArray<Background>& backgrounds)
      { this->background_system(r, scenes, drawables, backgrounds); });

  _registery.get().add_system<Scene, Position, Drawable, Sprite>(
      [this](Registery& r,
             SparseArray<Scene>& scenes,
             SparseArray<Position>& pos,
             SparseArray<Drawable>& draw,
             SparseArray<Sprite>& spr)
      { this->render_sprites(r, scenes, pos, draw, spr); });

  _registery.get().add_system<Scene, Position, Drawable, Text>(
      [this](Registery& r,
             const SparseArray<Scene>& scenes,
             const SparseArray<Position>& pos,
             const SparseArray<Drawable>& draw,
             const SparseArray<Text>& txt)
      { this->render_text(r, scenes, pos, draw, txt); });

  _registery.get().add_system<Scene, Position, Drawable, AnimatedSprite>(
      [this](Registery& r,
             const SparseArray<Scene>& scenes,
             const SparseArray<Position>& positions,
             const SparseArray<Drawable>& drawable,
             SparseArray<AnimatedSprite>& AnimatedSprites)
      {
        this->animation_system(r, scenes, positions, drawable, AnimatedSprites);
      });

  _registery.get().add_system<>([this](Registery&) { this->display(); });
  _textures.insert_or_assign(SFMLRenderer::placeholder_texture,
                             gen_placeholder());
}

SFMLRenderer::~SFMLRenderer()
{
  if (_window.isOpen()) {
    _window.close();
  }
}

sf::Texture& SFMLRenderer::load_texture(std::string const& path)
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

sf::Font& SFMLRenderer::load_font(std::string const& path)
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

void SFMLRenderer::init_drawable(Registery::Entity const& entity,
                                 JsonObject const&)
{
  _registery.get().emplace_component<Drawable>(entity);
}

void SFMLRenderer::init_sprite(Registery::Entity const& entity,
                               JsonObject const& obj)
{
  auto const& texture_path = get_value<Sprite, std::string>(
      this->_registery.get(), obj, entity, "texture");

  if (!texture_path) {
    std::cerr << "Error loading sprite component: unexpected value type "
                 "(texture: string)\n";
    return;
  }

  Vector2D scale(0.1, 0.1);
  if (obj.contains("size")) {
    scale = this->parse_vector2d<Sprite>(entity, obj, "size");
  }
  _registery.get().emplace_component<Sprite>(
      entity, texture_path.value(), scale);
}

void SFMLRenderer::init_text(Registery::Entity const& entity,
                             JsonObject const& obj)
{
  auto const& font_path =
      get_value<Text, std::string>(this->_registery.get(), obj, entity, "font");

  if (!font_path) {
    std::cerr << "Error loading text component: unexpected value type (font: "
                 "string)\n";
    return;
  }

  Vector2D scale(0.1, 0.1);
  if (obj.contains("size")) {
    scale = this->parse_vector2d<Text>(entity, obj, "size");
  }

  auto& text_opt = _registery.get().emplace_component<Text>(
      entity, font_path.value(), scale, "");

  if (text_opt.has_value()) {
    auto text_val = get_value<Text, std::string>(
        this->_registery.get(), obj, entity, "text");
    if (text_val) {
      text_opt.value().text = text_val.value();
    }
  }
}

std::optional<Key> SFMLRenderer::sfml_key_to_key(sf::Keyboard::Key sfml_key)
{
  auto it = key_association.find(sfml_key);
  if (it != key_association.end()) {
    return it->second;
  }
  return std::nullopt;
}

void SFMLRenderer::handle_resize()
{
  sf::Vector2u new_size = _window.getSize();
  this->_view.setSize(sf::Vector2f(static_cast<float>(new_size.x),
                                   static_cast<float>(new_size.y)));
  this->_view.setCenter(sf::Vector2f(static_cast<float>(new_size.x) / 2,
                                     static_cast<float>(new_size.y) / 2));
  _window.setView(this->_view);
}

void SFMLRenderer::handle_events()
{
  if (!_window.isOpen()) {
    return;
  }

  _key_pressed.key_pressed.clear();
  if (_key_pressed.key_unicode.has_value()) {
    _key_pressed.key_unicode->clear();
  }

  _key_released.key_released.clear();
  if (_key_released.key_unicode.has_value()) {
    _key_released.key_unicode->clear();
  }

  while (const std::optional event = _window.pollEvent()) {
    if (event->is<sf::Event::Closed>()) {
      _window.close();
      _registery.get().emit<ShutdownEvent>("Window closed", 0);
    }
    if (const auto* key_pressed = event->getIf<sf::Event::KeyPressed>()) {
      auto key = sfml_key_to_key(key_pressed->code);
      if (key.has_value()) {
        _key_pressed.key_pressed[key.value()] = true;
      }
    }
    if (const auto* key_released = event->getIf<sf::Event::KeyReleased>()) {
      auto key = sfml_key_to_key(key_released->code);
      if (key.has_value()) {
        _key_released.key_released[key.value()] = true;
      }
    }
    if (const auto* text_entered = event->getIf<sf::Event::TextEntered>()) {
      if (text_entered->unicode >= 'A' && text_entered->unicode < 'z') {
        if (!_key_pressed.key_unicode.has_value()) {
          _key_pressed.key_unicode = "";
        }
        _key_pressed.key_unicode.value() +=
            static_cast<char>(text_entered->unicode);
      }
    }
    if (event->is<sf::Event::Resized>()) {
      handle_resize();
    }
  }
  if (!_key_pressed.key_pressed.empty() || _key_pressed.key_unicode.has_value())
  {
    _registery.get().emit<KeyPressedEvent>(_key_pressed);
  }
  if (!_key_released.key_released.empty()) {
    _registery.get().emit<KeyReleasedEvent>(_key_released);
  }
}

void SFMLRenderer::display()
{
  if (!_window.isOpen()) {
    return;
  }
  _window.display();
}

void SFMLRenderer::render_sprites(Registery& /*unused*/,
                                  const SparseArray<Scene>& scenes,
                                  const SparseArray<Position>& positions,
                                  const SparseArray<Drawable>& drawable,
                                  const SparseArray<Sprite>& sprites)
{
  std::vector<
      std::
          tuple<std::reference_wrapper<sf::Texture>, double, sf::Vector2f, int>>
      drawables;
  sf::Vector2u window_size = _window.getSize();
  float min_dimension =
      static_cast<float>(std::min(window_size.x, window_size.y));

  drawables.reserve(
      std::max({positions.size(), drawable.size(), sprites.size()}));

  for (auto&& [scene, pos, draw, spr] :
       Zipper(scenes, positions, drawable, sprites))
  {
    if (!draw.enabled) {
      continue;
    }
    if (scene.state == SceneState::DISABLED) {
      continue;
    }

    sf::Texture& texture = load_texture(spr.texture_path);

    float scale_x =
        static_cast<float>(window_size.x * spr.scale.x) / texture.getSize().x;
    float scale_y =
        static_cast<float>(window_size.y * spr.scale.y) / texture.getSize().y;
    float uniform_scale = std::min(scale_x, scale_y);

    sf::Vector2f new_pos(
        static_cast<float>((pos.pos.x + 1.0) * min_dimension / 2.0f),
        static_cast<float>((pos.pos.y + 1.0) * min_dimension / 2.0f));
    drawables.emplace_back(std::ref(texture), uniform_scale, new_pos, pos.z);
  }
  std::sort(drawables.begin(),
            drawables.end(),
            [](auto const& a, auto const& b)
            { return std::get<3>(a) < std::get<3>(b); });

  for (auto&& [texture, scale, new_pos, z] : drawables) {
    if (!this->_sprite.has_value()) {
      this->_sprite.emplace(texture.get());
    } else {
      this->_sprite->setTexture(texture.get(), true);
    }
    this->_sprite->setOrigin(
        sf::Vector2f(static_cast<float>(texture.get().getSize().x) / 2.0f,
                     static_cast<float>(texture.get().getSize().y) / 2.0f));
    this->_sprite->setScale(sf::Vector2f(scale, scale));
    this->_sprite->setPosition(new_pos);
    _window.draw(*this->_sprite);
  }
}

void SFMLRenderer::render_text(Registery& /*unused*/,
                               const SparseArray<Scene>& scenes,
                               const SparseArray<Position>& positions,
                               const SparseArray<Drawable>& drawable,
                               const SparseArray<Text>& texts)
{
  for (auto&& [scene, pos, draw, txt] :
       Zipper(scenes, positions, drawable, texts))
  {
    if (!draw.enabled) {
      continue;
    }
    if (scene.state == SceneState::DISABLED) {
      continue;
    }

    sf::Font& font = load_font(txt.font_path);
    if (!_text.has_value()) {
      _text.emplace(font);
    }
    _text.value().setFont(font);

    _text.value().setString(txt.text);

    sf::Vector2u window_size = _window.getSize();
    float min_dimension =
        static_cast<float>(std::min(window_size.x, window_size.y));
    sf::Vector2f new_pos(
        static_cast<float>((pos.pos.x + 1.0) * min_dimension / 2.0f),
        static_cast<float>((pos.pos.y + 1.0) * min_dimension / 2.0f));
    _text.value().setPosition(new_pos);
    _text.value().setCharacterSize(static_cast<unsigned int>(txt.scale.x));
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
