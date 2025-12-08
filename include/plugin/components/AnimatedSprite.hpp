/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Animated_Sprite
*/

#ifndef ANIMATED_SPRITE_HPP_
#define ANIMATED_SPRITE_HPP_

#include <string>

#include "ByteParser/ByteParser.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"

struct AnimationData
{
  AnimationData(std::string texture_path,
                Vector2D frame_size,
                double framerate,
                int nb_frames,
                int current_frame,
                bool loop)
      : texture_path(std::move(texture_path))
      , frame_size(frame_size)
      , framerate(framerate)
      , nb_frames(nb_frames)
      , current_frame(current_frame)
      , loop(loop)
  {
  }

  std::string texture_path;
  Vector2D frame_size;
  double framerate;
  int nb_frames;
  int current_frame = 0;
  bool loop = false;

  DEFAULT_BYTE_CONSTRUCTOR(AnimationData,
                           (
                               [](std::vector<char> texture_path_vec,
                                  Vector2D frame_size,
                                  double framerate,
                                  int nb_frames,
                                  int current_frame,
                                  bool loop)
                               {
                                 return AnimationData(
                                     std::string(texture_path_vec.begin(),
                                                 texture_path_vec.end()),
                                     frame_size,
                                     framerate,
                                     nb_frames,
                                     current_frame,
                                     loop);
                               }),
                           parseByteArray(parseAnyChar()),
                           parseVector2D(),
                           parseByte<double>(),
                           parseByte<int>(),
                           parseByte<int>(),
                           parseByte<bool>())

  DEFAULT_SERIALIZE(string_to_byte(this->texture_path),
                    vector2DToByte(this->frame_size),
                    type_to_byte(this->framerate),
                    type_to_byte(this->nb_frames),
                    type_to_byte(this->current_frame),
                    type_to_byte(this->loop))

  HOOKABLE(AnimationData,
           HOOK(texture_path),
           HOOK(frame_size),
           HOOK(framerate),
           HOOK(nb_frames),
           HOOK(current_frame),
           HOOK(loop))
};

class AnimatedSprite
{
public:
  AnimatedSprite();
  ~AnimatedSprite();

  std::map<std::string, AnimationData> animations;
  std::string current_animation;
  std::string default_animation = "";


};

#endif /* !ANIMATED_SPRITE_HPP_ */
