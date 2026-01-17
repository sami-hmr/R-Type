#include <iostream>
#include <string>
#include <unordered_map>
#include <variant>

#include "plugin/components/AnimatedSprite.hpp"

#include "Json/JsonParser.hpp"
#include "NetworkShared.hpp"
#include "UI.hpp"
#include "ecs/EventManager.hpp"
#include "ecs/Registry.hpp"
#include "ecs/Scenes.hpp"
#include "ecs/zipper/ZipperIndex.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/components/Drawable.hpp"
#include "plugin/components/Health.hpp"
#include "plugin/components/Position.hpp"
#include "plugin/components/Speed.hpp"
#include "plugin/components/Text.hpp"
#include "plugin/events/EntityManagementEvent.hpp"
#include "plugin/events/LoggerEvent.hpp"

static const double deux = 2.0;

void AnimatedSprite::update_anim(
    Registry& /*r*/,
    EventManager& em,
    std::chrono::high_resolution_clock::time_point now,
    Ecs::Entity entity)
{
  AnimationData& animation = this->animations.at(this->current_animation);

  double elapsed = std::chrono::duration<double>(now - last_update).count();

  if (elapsed >= (1.0 / animation.framerate)) {
    if (animation.current_frame == 0) {
      em.emit<AnimationStartEvent>(this->current_animation, entity);
    }
    animation.current_frame += 1;
    animation.frame_pos += animation.direction * animation.frame_size;
    if (animation.current_frame >= animation.nb_frames
        || animation.current_frame < 0)
    {
      em.emit<AnimationEndEvent>(this->current_animation, entity);
      if (!animation.loop) {
        animation.current_frame = animation.nb_frames - 1;
        animation.frame_pos -= animation.direction * animation.frame_size;
        this->last_update = now;
        if (this->animations.contains("idle")) {
          this->current_animation = "idle";
          animation.current_frame = 0;
          animation.frame_pos = animation.initial_frame_pos;
        }
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

void AnimatedSprite::on_death(Registry& r,
                              EventManager& em,
                              const DeathEvent& event)
{
  if (r.is_entity_dying(event.entity)) {
    return;
  }
  if (!r.has_component<AnimatedSprite>(event.entity)) {
    em.emit<DeleteEntity>(event.entity);
    return;
  }

  auto& animated_sprites = r.get_components<AnimatedSprite>();

  if (animated_sprites[event.entity].value().animations.contains("death")
      && animated_sprites[event.entity]->current_animation != "death")
  {
    AnimationData& animdata =
        animated_sprites[event.entity].value().animations.at("death");
    em.emit<PlayAnimationEvent>(
        "death", event.entity, animdata.framerate, false, false);
    r.remove_component<Speed>(event.entity);
  } else {
    em.emit<DeleteEntity>(event.entity);
  }
}

void AnimatedSprite::on_animation_end(Registry& r,
                                      EventManager& em,
                                      const AnimationEndEvent& event)
{
  if (!r.has_component<AnimatedSprite>(event.entity)) {
    return;
  }

  if (event.name == "death") {
    em.emit<DeleteEntity>(event.entity);
  }
}

void AnimatedSprite::on_play_animation(Registry& r,
                                       EventManager& em,
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

  em.emit<AnimationStartEvent>(event.name, event.entity);
  animData.framerate = event.framerate;
  animData.loop = event.loop;
  animData.rollback = event.rollback;
  animSprite.current_animation = event.name;
  em.emit<ComponentBuilder>(event.entity,
                            r.get_component_key<AnimatedSprite>(),
                            animSprite.to_bytes());
}

void UI::update_anim_system(Registry& r)
{
  auto now = r.clock().now();

  for (auto&& [e, drawable, anim] : ZipperIndex<Drawable, AnimatedSprite>(r)) {
    if (!drawable.enabled) {
      continue;
    }
    anim.update_anim(r, this->_event_manager.get(), now, e);
    // this->_event_manager.get().emit<ComponentBuilder>(
    //     e,
    //     r.get_component_key<AnimatedSprite>(),
    //     anim.to_bytes());
  }
}


std::optional<AnimationData> UI::parse_animation_data(JsonObject const& obj,
                                                      Ecs::Entity const& e)
{
  AnimationData animdata;

  auto const& texture_path = get_value<AnimatedSprite, std::string>(
      this->_registry.get(), obj, e, "texture");
  if (!texture_path) {
    std::cerr << "Error parsing animation data: \"texture\" field not "
                 "found or invalid"
              << "\n";
    return std::nullopt;
  }
  animdata.texture_path = texture_path.value();

  animdata.frame_size = get_value<AnimatedSprite, Vector2D>(
                            this->_registry.get(), obj, e, "frame_size")
                            .value();

  animdata.frame_pos = get_value<AnimatedSprite, Vector2D>(
                           this->_registry.get(), obj, e, "frame_pos")
                           .value();

  auto const& framerate = get_value<AnimatedSprite, double>(
      this->_registry.get(), obj, e, "framerate");
  if (!framerate) {
    std::cerr << "Error parsing animation data: \"framerate\" field not found "
                 "or invalid"
              << "\n";
    return std::nullopt;
  }
  animdata.direction = get_value<AnimatedSprite, Vector2D>(
                           this->_registry.get(), obj, e, "direction")
                           .value();

  animdata.framerate = framerate.value();
  animdata.sprite_size =
      get_value<Text, Vector2D>(
          this->_registry.get(), obj, e, "sprite_size", "width", "height")
          .value();

  auto const& nb_frames = get_value<AnimatedSprite, int>(
      this->_registry.get(), obj, e, "nb_frames");
  if (!nb_frames) {
    std::cerr << "Error parsing animation data: \"nb_frames\" field not found "
                 "or invalid"
              << "\n";
    return std::nullopt;
  }
  animdata.nb_frames = nb_frames.value();
  auto const& loop =
      get_value<AnimatedSprite, bool>(this->_registry.get(), obj, e, "loop");
  if (!loop) {
    std::cerr
        << "Error parsing animation data: \"loop\" field not found or invalid"
        << "\n";
    return std::nullopt;
  }
  animdata.loop = loop.value();
  auto const& rollback = get_value<AnimatedSprite, bool>(
      this->_registry.get(), obj, e, "rollback");
  if (!rollback) {
    std::cerr << "Error parsing animation data: \"rollback\" field not found "
                 "or " "invalid"
              << "\n";
    return std::nullopt;
  }
  animdata.rollback = rollback.value();
  return animdata;
}

void UI::init_animated_sprite(Ecs::Entity const& entity,
                              const JsonObject& obj)
{
  std::unordered_map<std::string, AnimationData> animations;

  std::optional<JsonArray> animations_obj =
      get_value<AnimatedSprite, JsonArray>(
          this->_registry.get(), obj, entity, "animations");

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
      this->_registry.get(), obj, entity, "default_animation");
  if (default_animation_value) {
    default_animation = default_animation_value.value();
  }
  init_component<AnimatedSprite>(this->_registry.get(),
                                 this->_event_manager.get(),
                                 entity,
                                 std::move(animations),
                                 default_animation,
                                 default_animation);
}
