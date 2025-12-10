#include <iostream>
#include <string>
#include <unordered_map>
#include <variant>

#include "plugin/components/AnimatedSprite.hpp"

#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>

#include "Json/JsonParser.hpp"
#include "SFMLRenderer.hpp"
#include "ecs/Registry.hpp"
#include "ecs/Scenes.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/Hooks.hpp"

void SFMLRenderer::animation_system(
    Registry& r,
    const SparseArray<Scene>& scenes,
    const SparseArray<Position>& positions,
    const SparseArray<Drawable>& drawable,
    SparseArray<AnimatedSprite>& AnimatedSprites)
{
  auto now = std::chrono::high_resolution_clock::now();

  std::vector<std::tuple<std::reference_wrapper<sf::Texture>,
                         double,
                         sf::Vector2f,
                         int,
                         AnimationData>>
      drawables;

  drawables.reserve(AnimatedSprites.size());
  sf::Vector2u window_size = _window.getSize();

  for (auto&& [scene, pos, draw, anim] :
       Zipper(scenes, positions, drawable, AnimatedSprites))
  {
    if (scene.state == SceneState::DISABLED) {
      continue;
    }
    if (!draw.enabled) {
      continue;
    }
    if (!anim.animations.contains(anim.current_animation)) {
      continue;
    }
    anim.update_anim(now);
    AnimationData anim_data = anim.animations.at(anim.current_animation);

    sf::Texture& texture = load_texture(anim_data.texture_path);

    float scale_x = static_cast<float>(window_size.x * anim_data.sprite_size.x)
        / anim_data.frame_size.x;
    float scale_y = static_cast<float>(window_size.y * anim_data.sprite_size.y)
        / anim_data.frame_size.y;
    float uniform_scale = std::min(scale_x, scale_y);

    sf::Vector2f new_pos(
        static_cast<float>((pos.pos.x + 1.0) * window_size.x / 2.0),
        static_cast<float>((pos.pos.y + 1.0) * window_size.y / 2.0));
    drawables.emplace_back(
        std::ref(texture), uniform_scale, new_pos, pos.z, anim_data);
  }
  std::sort(drawables.begin(),
            drawables.end(),
            [](auto const& a, auto const& b)
            { return std::get<3>(a) < std::get<3>(b); });
  for (auto&& [texture, scale, new_pos, z, anim_data] : drawables) {
    if (!this->_sprite.has_value()) {
      this->_sprite.emplace(texture.get());
    } else {
      this->_sprite->setTexture(texture.get(), true);
    }
    this->_sprite->setOrigin(
        sf::Vector2f(static_cast<float>(anim_data.frame_size.x) / 2.0f,
                     static_cast<float>(anim_data.frame_size.y) / 2.0f));
    this->_sprite->setTextureRect(
        sf::IntRect({static_cast<int>(anim_data.frame_pos.x),
                     static_cast<int>(anim_data.frame_pos.y)},
                    {
                        static_cast<int>(anim_data.frame_size.x),
                        static_cast<int>(anim_data.frame_size.y),
                    }));
    this->_sprite->setScale(
        {static_cast<float>(scale), static_cast<float>(scale)});
    this->_sprite->setPosition(new_pos);
    _window.draw(*this->_sprite);
  }
}

void AnimatedSprite::update_anim(
    std::chrono::high_resolution_clock::time_point now)
{
  AnimationData& animation = this->animations.at(this->current_animation);

  double elapsed = std::chrono::duration<double>(now - last_update).count();

  if (elapsed >= (1.0 / animation.framerate)) {
    animation.current_frame += 1;
    animation.frame_pos += animation.direction * animation.frame_size;
    if (animation.current_frame >= animation.nb_frames
        || animation.current_frame < 0)
    {
      if (animation.rollback) {
        animation.direction = animation.direction * -1;
        animation.frame_pos += animation.direction * animation.frame_size;
      } else {
        animation.frame_pos = Vector2D(0, 0);
      }
      animation.current_frame = 0;
      if (!animation.loop) {
        this->current_animation = default_animation;
      }
    }
    this->last_update = now;
  }
}
