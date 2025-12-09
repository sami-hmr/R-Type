#include <iostream>
#include <string>
#include <unordered_map>
#include <variant>

#include "plugin/components/AnimatedSprite.hpp"

#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>

#include "Json/JsonParser.hpp"
#include "SFMLRenderer.hpp"
#include "ecs/Registery.hpp"
#include "ecs/Scenes.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/Hooks.hpp"

std::optional<AnimationData> SFMLRenderer::parse_animation_data(
    JsonObject const& obj, Registery::Entity const& e)
{
  AnimationData animdata;

  auto const& texture_path = get_value<AnimatedSprite, std::string>(
      this->_registery.get(), obj, e, "texture");
  if (!texture_path) {
    std::cerr << "Error parsing animation data: \"texture\" field not "
                 "found or invalid"
              << "\n";
    return std::nullopt;
  }
  animdata.texture_path = texture_path.value();

  animdata.frame_size = get_value<AnimatedSprite, Vector2D>(
                            this->_registery.get(), obj, e, "frame_size")
                            .value();

  animdata.frame_pos = get_value<AnimatedSprite, Vector2D>(
                           this->_registery.get(), obj, e, "frame_pos")
                           .value();

  auto const& framerate = get_value<AnimatedSprite, double>(
      this->_registery.get(), obj, e, "framerate");
  if (!framerate) {
    std::cerr << "Error parsing animation data: \"framerate\" field not found "
                 "or invalid"
              << "\n";
    return std::nullopt;
  }
  animdata.direction = get_value<AnimatedSprite, Vector2D>(
                           this->_registery.get(), obj, e, "direction")
                           .value();

  animdata.framerate = framerate.value();
  animdata.sprite_size = parse_vector2d<AnimatedSprite>(e, obj, "sprite_size");

  auto const& nb_frames = get_value<AnimatedSprite, int>(
      this->_registery.get(), obj, e, "nb_frames");
  if (!nb_frames) {
    std::cerr << "Error parsing animation data: \"nb_frames\" field not found "
                 "or invalid"
              << "\n";
    return std::nullopt;
  }
  animdata.nb_frames = nb_frames.value();
  auto const& loop =
      get_value<AnimatedSprite, bool>(this->_registery.get(), obj, e, "loop");
  if (!loop) {
    std::cerr
        << "Error parsing animation data: \"loop\" field not found or invalid"
        << "\n";
    return std::nullopt;
  }
  animdata.loop = loop.value();
  auto const& rollback = get_value<AnimatedSprite, bool>(
      this->_registery.get(), obj, e, "rollback");
  if (!rollback) {
    std::cerr << "Error parsing animation data: \"rollback\" field not found or "
                 "invalid"
              << "\n";
    return std::nullopt;
  }
  animdata.rollback = rollback.value();
  return animdata;
}

void SFMLRenderer::init_animated_sprite(Registery::Entity const& entity,
                                        const JsonObject& obj)
{
  std::unordered_map<std::string, AnimationData> animations;

  std::optional<JsonArray> animations_obj =
      get_value<AnimatedSprite, JsonArray>(
          this->_registery.get(), obj, entity, "animations");

  if (!animations_obj) {
    std::cerr << "AnimatedSprite component requires animations array"
              << "\n";
    return;
  }
  for (const JsonValue& animation_value : animations_obj.value()) {
    std::optional<AnimationData> animdata;
    std::string name;
    JsonObject animdata_obj;
    try {
      animdata_obj = std::get<JsonObject>(animation_value.value);
    } catch (std::bad_variant_access const&) {
      std::cerr << "Error parsing animation data: not a JsonObject" << '\n';
      return;
    }
    if (!animdata_obj.contains("name")) {
      std::cerr << "Error parsing animation data: \"name\" field not found"
                << '\n';
      return;
    }
    try {
      name = std::get<std::string>(animdata_obj.at("name").value);
    } catch (std::bad_variant_access const&) {
      std::cerr
          << "Error parsing animation data: \"name\" field is not a string"
          << '\n';
      return;
    }
    animdata = parse_animation_data(animdata_obj, entity);

    if (!animdata) {
      std::cerr << "Error parsing animation data for animation: " << name
                << '\n';
      return;
    }
    animations.insert_or_assign(name, animdata.value());
  }
  std::string default_animation = animations.begin()->first;
  auto const& default_animation_value = get_value<AnimatedSprite, std::string>(
      this->_registery.get(), obj, entity, "default_animation");
  if (default_animation_value) {
    default_animation = default_animation_value.value();
  }
  _registery.get().emplace_component<AnimatedSprite>(
      entity, std::move(animations), default_animation, default_animation);
}

void SFMLRenderer::animation_system(
    Registery& r,
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
    if (animation.current_frame >= animation.nb_frames || animation.current_frame < 0) {
      if (animation.rollback) {
        animation.direction = animation.direction * -1;
        animation.frame_pos += animation.direction * animation.frame_size;
        animation.current_frame = 0;
      } else {
        animation.current_frame = 0;
        animation.frame_pos = Vector2D(0, 0);
      }
      if (!animation.loop) {
        this->current_animation = default_animation;
      }
    }
    this->last_update = now;
  }
}
