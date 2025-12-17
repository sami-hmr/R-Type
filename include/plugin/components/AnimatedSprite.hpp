/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Animated_Sprite
*/

#ifndef ANIMATED_SPRITE_HPP_
#define ANIMATED_SPRITE_HPP_

#include <string>
#include <unordered_map>

#include "BaseTypes.hpp"
#include "ByteParser/ByteParser.hpp"
#include "ecs/Registry.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/events/AnimationEvents.hpp"
#include "plugin/events/DeathEvent.hpp"
#include "plugin/events/DamageEvent.hpp"

struct AnimationData
{
  AnimationData() = default;

  AnimationData(std::string texture_path,
                Vector2D frame_size,
                Vector2D frame_pos,
                Vector2D direction,
                Vector2D sprite_size,
                double framerate,
                int nb_frames,
                int current_frame,
                bool loop,
                bool rollback)
      : texture_path(std::move(texture_path))
      , frame_size(frame_size)
      , frame_pos(frame_pos)
      , initial_frame_pos(frame_pos)
      , direction(direction)
      , sprite_size(sprite_size)
      , framerate(framerate)
      , nb_frames(nb_frames)
      , current_frame(current_frame)
      , loop(loop)
      , rollback(rollback)
  {
  }

  std::string texture_path;
  Vector2D frame_size;
  Vector2D frame_pos;
  Vector2D initial_frame_pos;
  Vector2D direction;
  Vector2D sprite_size;
  double framerate = 0;
  int nb_frames = 0;
  int current_frame = 0;
  bool loop = false;
  bool rollback = false;


  DEFAULT_BYTE_CONSTRUCTOR(AnimationData,
                           (
                               [](std::string texture,
                                  Vector2D frame_size,
                                  Vector2D frame_pos,
                                  Vector2D direction,
                                  Vector2D sprite_size,
                                  double framerate,
                                  int nb_frames,
                                  int current_frame,
                                  bool loop,
                                  bool rollback)
                               {
                                 return AnimationData(
                                     texture,
                                     frame_size,
                                     frame_pos,
                                     direction,
                                     sprite_size,
                                     framerate,
                                     nb_frames,
                                     current_frame,
                                     loop,
                                     rollback);
                               }),
                           parseByteString(),
                           parseVector2D(),
                           parseVector2D(),
                           parseVector2D(),
                           parseVector2D(),
                           parseByte<double>(),
                           parseByte<int>(),
                           parseByte<int>(),
                           parseByte<bool>(),
                           parseByte<bool>())

  DEFAULT_SERIALIZE(string_to_byte(this->texture_path),
                    vector2DToByte(this->frame_size),
                    vector2DToByte(this->frame_pos),
                    vector2DToByte(this->direction),
                    vector2DToByte(this->sprite_size),
                    type_to_byte(this->framerate),
                    type_to_byte(this->nb_frames),
                    type_to_byte(this->current_frame),
                    type_to_byte(this->loop),
                    type_to_byte(this->rollback))

  HOOKABLE(AnimationData,
           HOOK(texture_path),
           HOOK(frame_size),
           HOOK(frame_pos),
           HOOK(direction),
           HOOK(sprite_size),
           HOOK(framerate),
           HOOK(nb_frames),
           HOOK(current_frame),
           HOOK(loop),
           HOOK(rollback))
};

inline Parser<AnimationData> parseAnimationData()
{
  return apply(
      [](std::string texture_path_vec,
         Vector2D frame_size,
         Vector2D frame_pos,
         Vector2D direction,
         Vector2D sprite_size,
         double framerate,
         int nb_frames,
         int current_frame,
         bool loop,
         bool rollback)
      {
        return AnimationData(
            std::string(texture_path_vec.begin(), texture_path_vec.end()),
            frame_size,
            frame_pos,
            direction,
            sprite_size,
            framerate,
            nb_frames,
            current_frame,
            loop,
            rollback);
      },
      parseByteString(),
      parseVector2D(),
      parseVector2D(),
      parseVector2D(),
      parseVector2D(),
      parseByte<double>(),
      parseByte<int>(),
      parseByte<int>(),
      parseByte<bool>(),
      parseByte<bool>());
}

class AnimatedSprite
{
public:
  AnimatedSprite(std::unordered_map<std::string, AnimationData> animations,
                 std::string current_animation,
                 std::string default_animation)
      : animations(std::move(animations))
      , current_animation(std::move(current_animation))
      , default_animation(std::move(default_animation))
  {
    this->last_update = std::chrono::high_resolution_clock::now();
  };

  ~AnimatedSprite() = default;

  std::unordered_map<std::string, AnimationData> animations;
  std::string current_animation;
  std::string default_animation = "";

  std::chrono::high_resolution_clock::time_point last_update;

  void update_anim(Registry& r,
                   std::chrono::high_resolution_clock::time_point now,
                   int entity);
  static void on_death(Registry& r, const DeathEvent& event);
  static void on_animation_end(Registry& r, const AnimationEndEvent& event);
  static void on_play_animation(Registry& r, const PlayAnimationEvent& event);

  DEFAULT_BYTE_CONSTRUCTOR(
      AnimatedSprite,
      (
          [](std::unordered_map<std::string, AnimationData> animations,
             std::string current_animation,
             std::string default_animation)
          {
            return AnimatedSprite(std::move(animations),
                                  std::move(current_animation),
                                  std::move(default_animation));
          }),
      parseByteMap(parseByteString(), parseAnimationData()),
      parseByteString(),
      parseByteString())

  CHANGE_ENTITY_DEFAULT

  DEFAULT_SERIALIZE(
      map_to_byte<std::string, AnimationData>(
          this->animations,
          std::function<ByteArray(std::string const&)>(string_to_byte),
          std::function<ByteArray(AnimationData)>([](AnimationData data)
                                                  { return data.to_bytes(); })),
      string_to_byte(this->current_animation),
      string_to_byte(this->default_animation))

  HOOKABLE(AnimatedSprite,
           HOOK(animations),
           HOOK(current_animation),
           HOOK(default_animation))
};

#endif /* !ANIMATED_SPRITE_HPP_ */
