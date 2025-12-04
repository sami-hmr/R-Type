#include <iostream>

#include "plugin/components/Background.hpp"

#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Vector2.hpp>

#include "Json/JsonParser.hpp"
#include "SFMLRenderer.hpp"
#include "ecs/Registery.hpp"
#include "ecs/zipper/Zipper.hpp"

void SFMLRenderer::init_background(Registery::Entity const entity,
                                   JsonObject const& obj)
{
  auto const& textures_path =
      get_value<JsonArray>(this->_registery.get(), obj, "layers");

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
    const auto& path_str =
        get_value<std::string>(this->_registery.get(), path_obj, "path");

    if (path_str.has_value()) {
      std::cout << "Adding background layer: " << path_str.value() << "\n";
      paths.push_back(path_str.value());
    }
  }
  Background::RenderType render_type = Background::RenderType::NOTHING;

  auto const& render_type_str =
      get_value<std::string>(this->_registery.get(), obj, "render_type");
  if (render_type_str.has_value()) {
    if (render_type_map.contains(render_type_str.value())) {
      render_type = render_type_map.at(render_type_str.value());
    } else {
      std::cerr << "Error loading Background component: invalid render_type "
                   "value, using default (NOTHING)\n";
    }
  }
  bool parallax = false;
  auto const& parallax_value =
      get_value<bool>(this->_registery.get(), obj, "parallax");
  if (parallax_value.has_value()) {
    parallax = parallax_value.value();
  }
  this->_registery.get().emplace_component<Background>(
      entity, Background(paths, render_type, parallax));
}

void SFMLRenderer::background_system(Registery& r,
                                     const SparseArray<Drawable>& drawables,
                                     const SparseArray<Background>& backgrounds)
{
  for (const auto&& [draw, background] : Zipper(drawables, backgrounds)) {
    if (!draw.enabled) {
      continue;
    }
    this->_draw_functions.at(background.render_type)(background);
  }
}

void SFMLRenderer::draw_nothing_background(const Background& bg)
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

void SFMLRenderer::draw_parallax_background(const Background& bg) {}

/**
 * @brief if the background does not fill the window, draw it again
 *
 * @param bg
 */
void SFMLRenderer::draw_repeat_background(const Background& bg)
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
void SFMLRenderer::draw_stretch_background(const Background& bg)
{
  sf::Vector2u window_size = _window.getSize();

  for (const std::string& texture_path : bg.textures_path) {
    sf::Texture& texture = load_texture(texture_path);

    if (!this->_sprite.has_value()) {
      this->_sprite.emplace(texture);
    } else {
      this->_sprite->setTexture(texture, true);
    }
    float scale_x = static_cast<float>(window_size.x) / texture.getSize().x;
    float scale_y = static_cast<float>(window_size.y) / texture.getSize().y;

    this->_sprite->setScale({scale_x, scale_y});
    this->_sprite->setOrigin({0, 0});
    this->_sprite->setPosition({0, 0});
    this->_window.draw(*this->_sprite);
  }
}
