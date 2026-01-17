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
