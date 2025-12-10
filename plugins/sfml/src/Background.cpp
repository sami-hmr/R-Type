#include <cmath>
#include <iostream>

#include "plugin/components/Background.hpp"

#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Vector2.hpp>

#include "Json/JsonParser.hpp"
#include "SFMLRenderer.hpp"
#include "ecs/Registry.hpp"
#include "ecs/Scenes.hpp"
#include "ecs/zipper/Zipper.hpp"
#include "libs/Vector2D.hpp"

void SFMLRenderer::init_background(Registry::Entity const& entity,
                                   JsonObject const& obj)
{
  auto const& textures_path = get_value<Background, JsonArray>(
      this->_registry.get(), obj, entity, "layers");

  if (!textures_path) {
    std::cerr << "Error loading Background component: unexpected value type "
                 "(expected texture: string)\n";
    return;
  }
  std::vector<std::string> paths;

  for (const auto& layer : textures_path.value()) {
    JsonObject path_obj;
    try {
      path_obj = std::get<JsonObject>(layer.value);
    } catch (std::bad_variant_access const&) {
      std::cerr << "Error loading Background component: unexpected layer "
                     "value type (expected JsonObject)\n";
      continue;
    }
    const auto& path_str = get_value<Background, std::string>(
        this->_registry.get(), path_obj, entity, "path");

    if (path_str.has_value()) {
      std::cout << "Adding background layer: " << path_str.value() << "\n";
      paths.push_back(path_str.value());
    }
  }
  Background::RenderType render_type = Background::RenderType::NOTHING;

  auto const& render_type_str = get_value<Background, std::string>(
      this->_registry.get(), obj, entity, "render_type");
  if (render_type_str.has_value()) {
    if (render_type_map.contains(render_type_str.value())) {
      render_type = render_type_map.at(render_type_str.value());
    } else {
      std::cerr << "Error loading Background component: invalid render_type "
                   "value, using default (NOTHING)\n";
    }
  }
  Parallax parallax;
  const std::optional<JsonObject>& parallax_obj =
      get_value<Background, JsonObject>(
          this->_registry.get(), obj, entity, "parallax");

  if (parallax_obj.has_value()) {
    const auto& active = get_value<Background, bool>(
        this->_registry.get(), parallax_obj.value(), entity, "active");
    const auto& speed = get_value<Background, Vector2D>(
        this->_registry, parallax_obj.value(), entity, "speed");
    const auto& framerate = get_value<Background, double>(
        this->_registry.get(), parallax_obj.value(), entity, "framerate");

    if (active.has_value() && framerate.has_value() && speed.has_value()) {
      parallax.speed = Vector2D(speed.value());
      parallax.active = active.value();
      parallax.framerate = framerate.value();
    } else {
      std::cerr << "Error loading Background component: invalid parallax "
                   "value, using default (inactive)\n";
    }
  }
  this->_registry.get().emplace_component<Background>(
      entity, Background(paths, render_type, parallax));
}

void SFMLRenderer::background_system(Registry& r,
                                     const SparseArray<Scene>& scenes,
                                     const SparseArray<Drawable>& drawables,
                                     SparseArray<Background>& backgrounds)
{
  for (const auto&& [scene, draw, background] :
       Zipper(scenes, drawables, backgrounds))
  {
    if (scene.state == SceneState::DISABLED) {
      continue;
    }
    if (!draw.enabled) {
      continue;
    }
    if (background.parallax.active) {
      double dt = r.clock().delta_seconds();
      background.parallax.pos.x += background.parallax.speed.x * dt;
      background.parallax.pos.y += background.parallax.speed.y * dt;
    }
    this->_draw_functions.at(background.render_type)(background);
  }
}

void SFMLRenderer::draw_nothing_background(Background& bg)
{
  for (const std::string& texture_path : bg.textures_path) {
    sf::Texture& texture = load_texture(texture_path);

    if (!this->_sprite.has_value()) {
      this->_sprite.emplace(texture);
    } else {
      this->_sprite->setTexture(texture, true);
    }
    this->_sprite->setScale({1, 1});
    this->_sprite->setOrigin({0, 0});
    this->_sprite->setPosition({0, 0});
    this->_window.draw(*this->_sprite);
  }
}

/**
 * @brief if the background does not fill the window, draw it again
 *
 * @param bg
 */
void SFMLRenderer::draw_repeat_background(Background& bg)
{
  sf::Vector2u window_size = _window.getSize();

  for (const std::string& texture_path : bg.textures_path) {
    sf::Texture& texture = load_texture(texture_path);

    if (!this->_sprite.has_value()) {
      this->_sprite.emplace(texture);
    } else {
      this->_sprite->setTexture(texture, true);
    }
    this->_sprite->setScale({1, 1});
    this->_sprite->setOrigin({0, 0});

    unsigned int tex_width = texture.getSize().x;
    unsigned int tex_height = texture.getSize().y;
    if (tex_width == 0 || tex_height == 0) {
      continue;
    }

    unsigned int tiles_x = (window_size.x + tex_width - 1) / tex_width;
    unsigned int tiles_y = (window_size.y + tex_height - 1) / tex_height;

    for (unsigned int y = 0; y < tiles_y; ++y) {
      for (unsigned int x = 0; x < tiles_x; ++x) {
        this->_sprite->setPosition({static_cast<float>(x * tex_width),
                                    static_cast<float>(y * tex_height)});
        this->_window.draw(*this->_sprite);
      }
    }
  }
}

/**
 * @brief Adapts the background to the window size by stretching it
 *
 * @param bg
 */
void SFMLRenderer::draw_stretch_background(Background& bg)
{
  sf::Vector2u window_size = _window.getSize();
  int i = bg.textures_path.size();

  for (const std::string& texture_path : bg.textures_path) {
    sf::Texture& texture = load_texture(texture_path);

    if (!this->_sprite.has_value()) {
      this->_sprite.emplace(texture);
    } else {
      this->_sprite->setTexture(texture, true);
    }
    float scale_x = static_cast<float>(window_size.x) / texture.getSize().x;
    float scale_y = static_cast<float>(window_size.y) / texture.getSize().y;
    float scaled_width = static_cast<float>(texture.getSize().x) * scale_x;
    float scaled_height = static_cast<float>(texture.getSize().y) * scale_y;

    this->_sprite->setScale({scale_x, scale_y});
    this->_sprite->setOrigin({0, 0});

    float offset_x =
        std::fmod(static_cast<float>(bg.parallax.pos.x / i), scaled_width);
    float offset_y =
        std::fmod(static_cast<float>(bg.parallax.pos.y / i), scaled_height);

    sf::Vector2f new_pos = {offset_x, offset_y};
    this->_sprite->setPosition(new_pos);
    this->_window.draw(*this->_sprite);
    if (bg.parallax.active) {
      sf::Vector2f tile_pos = new_pos;
      while (tile_pos.x > -scaled_width) {
        tile_pos.x -= scaled_width;
        if (tile_pos.x + scaled_width > 0) {
          this->_sprite->setPosition(tile_pos);
          this->_window.draw(*this->_sprite);
        }
      }
      tile_pos = new_pos;
      while (tile_pos.x < window_size.x) {
        tile_pos.x += scaled_width;
        if (tile_pos.x < window_size.x) {
          this->_sprite->setPosition(tile_pos);
          this->_window.draw(*this->_sprite);
        }
      }
    }
    i = std::max(i - 1, 1);
  }
}
