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
#include "ecs/zipper/ZipperIndex.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/components/Health.hpp"
#include "plugin/events/LoggerEvent.hpp"

void SFMLRenderer::animation_system(
    Registry& /*unused*/,
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

  sf::Vector2u window_size = _window.getSize();
  float min_dimension =
      static_cast<float>(std::min(window_size.x, window_size.y));
  sf::Vector2f view_size = this->_view.getSize();
  sf::Vector2f view_pos = this->_view.getCenter();

  drawables.reserve(AnimatedSprites.size());

  for (auto&& [entity, scene, pos, draw, anim] :
       ZipperIndex(scenes, positions, drawable, AnimatedSprites))
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
    sf::Vector2f new_pos(
        static_cast<float>((pos.pos.x + 1.0) * min_dimension / 2.0),
        static_cast<float>((pos.pos.y + 1.0) * min_dimension / 2.0));
    if (new_pos.x < view_pos.x - (view_size.x / 2)
        || new_pos.x > view_pos.x + (view_size.x / 2))
    {
      continue;
    }
    if (new_pos.y < view_pos.y - (view_size.y / 2)
        || new_pos.y > view_pos.y + (view_size.y / 2))
    {
      continue;
    }

    anim.update_anim(this->_registry.get(), now, entity);
    AnimationData anim_data = anim.animations.at(anim.current_animation);

    sf::Texture& texture = load_texture(anim_data.texture_path);

    float scale_x = static_cast<float>(min_dimension * anim_data.sprite_size.x)
        / anim_data.frame_size.x;
    float scale_y = static_cast<float>(min_dimension * anim_data.sprite_size.y)
        / anim_data.frame_size.y;
    float uniform_scale = std::min(scale_x, scale_y);

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
    Registry& r, std::chrono::high_resolution_clock::time_point now, int entity)
{
  AnimationData& animation = this->animations.at(this->current_animation);

  double elapsed = std::chrono::duration<double>(now - last_update).count();

  if (elapsed >= (1.0 / animation.framerate)) {
    if (animation.current_frame == 0) {
      r.emit<AnimationStartEvent>(this->current_animation, entity);
    }
    animation.current_frame += 1;
    animation.frame_pos += animation.direction * animation.frame_size;
    if (animation.current_frame >= animation.nb_frames
        || animation.current_frame < 0)
    {
      r.emit<AnimationEndEvent>(this->current_animation, entity);
      if (!animation.loop) {
        animation.current_frame = animation.nb_frames - 1;
        animation.frame_pos -= animation.direction * animation.frame_size;
        this->last_update = now;
        return;
      }
      if (animation.rollback) {
        animation.direction = animation.direction * -1;
        animation.frame_pos += animation.direction * animation.frame_size;
      } else {
        animation.frame_pos = animation.initial_frame_pos;
      }
      animation.current_frame = 0;
    }
    this->last_update = now;
  }
}

void AnimatedSprite::on_death(Registry& r, const DamageEvent& event)
{
  if (!r.has_component<AnimatedSprite>(event.target)
      || !r.has_component<Health>(event.target))
  {
    return;
  }

  auto& animated_sprites = r.get_components<AnimatedSprite>();
  auto& healths = r.get_components<Health>();

  if (healths[event.target]->current <= 0) {
    if (animated_sprites[event.target].value().animations.contains("death")) {
      AnimationData& animdata =
          animated_sprites[event.target].value().animations.at("death");
      r.emit<PlayAnimationEvent>(
          "death", event.target, animdata.framerate, false, false);
    } else {
      r.kill_entity(event.target);
    }
  }
}

void AnimatedSprite::on_animation_end(Registry& r,
                                      const AnimationEndEvent& event)
{
  if (!r.has_component<AnimatedSprite>(event.entity)) {
    return;
  }

  if (event.name == "death") {
    r.kill_entity(event.entity);
  }
}

void AnimatedSprite::on_play_animation(Registry& r,
                                       const PlayAnimationEvent& event)
{
  if (!r.has_component<AnimatedSprite>(event.entity)) {
    return;
  }

  auto& animatedSprites = r.get_components<AnimatedSprite>();
  AnimatedSprite& animSprite = animatedSprites[event.entity].value();

  if (!animSprite.animations.contains(event.name)) {
    return;
  }
  AnimationData& animData = animSprite.animations.at(event.name);

  r.emit<AnimationStartEvent>(event.name, event.entity);
  animData.framerate = event.framerate;
  animData.loop = event.loop;
  animData.rollback = event.rollback;
  animSprite.current_animation = event.name;
}
