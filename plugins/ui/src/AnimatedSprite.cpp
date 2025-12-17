#include <iostream>
#include <string>
#include <unordered_map>
#include <variant>

#include "plugin/components/AnimatedSprite.hpp"

#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/System/Vector2.hpp>

#include "Json/JsonParser.hpp"
#include "NetworkShared.hpp"
#include "UI.hpp"
#include "ecs/Registry.hpp"
#include "ecs/Scenes.hpp"
#include "ecs/zipper/ZipperIndex.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/components/Drawable.hpp"
#include "plugin/components/Health.hpp"
#include "plugin/components/Position.hpp"
#include "plugin/events/LoggerEvent.hpp"
#include "plugin/events/EntityManagementEvent.hpp"

static const double deux = 2.0;


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

void AnimatedSprite::on_death(Registry& r, const DeathEvent& event)
{

  if (r.is_entity_dying(event.entity)) {
    return;
  }
  if (!r.has_component<AnimatedSprite>(event.entity)) {
    r.emit<DeleteEntity>(event.entity);
    return;
  }

  auto& animated_sprites = r.get_components<AnimatedSprite>();

  if (animated_sprites[event.entity].value().animations.contains("death")) {
    AnimationData& animdata =
        animated_sprites[event.entity].value().animations.at("death");
    r.emit<PlayAnimationEvent>(
        "death", event.entity, animdata.framerate, false, false);
  } else {
    r.emit<DeleteEntity>(event.entity);
  }
}

void AnimatedSprite::on_animation_end(Registry& r,
                                      const AnimationEndEvent& event)
{
  if (!r.has_component<AnimatedSprite>(event.entity)) {
    return;
  }

  if (event.name == "death") {
    r.emit<DeleteEntity>(event.entity);
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
  r.emit<ComponentBuilder>(
      event.entity,
      r.get_component_key<AnimatedSprite>(),
      animSprite.to_bytes());
}

void UI::update_anim_system(Registry &r)
{
  auto now = r.clock().now();
 
  for (auto &&[e, drawable, anim] : ZipperIndex<Drawable, AnimatedSprite>(r)) {
    if (!drawable.enabled) {
      continue;
    }
    anim.update_anim(r, now, e);
    // r.emit<ComponentBuilder>(
    //     e,
    //     r.get_component_key<AnimatedSprite>(),
    //     anim.to_bytes());
  }
}