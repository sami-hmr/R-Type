#pragma once

#include <string>
#include <utility>

#include "BaseTypes.hpp"
#include "ByteParser/ByteParser.hpp"
#include "plugin/Byte.hpp"
#include "plugin/HookMacros.hpp"
#include "plugin/events/EventMacros.hpp"

struct SoundEffect
{
  std::string filepath;
  double volume;
  double pitch = 1.0;
  bool loop = false;
  bool play = false;
  bool stop = true;
  bool playing = false;

  SoundEffect() = default;

  SoundEffect(std::string filepath,
              double volume,
              double pitch = 1.0,
              bool loop = false,
              bool play = false,
              bool stop = true,
              bool playing = false)
      : filepath(std::move(filepath))
      , volume(volume)
      , pitch(pitch)
      , loop(loop)
      , play(play)
      , stop(stop)
      , playing(playing)
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(
      SoundEffect,
      [](std::string filepath,
         double volume,
         double pitch,
         bool loop,
         bool play,
         bool stop,
         bool playing)
      {
        return SoundEffect(
            std::move(filepath), volume, pitch, loop, play, stop, playing);
      },
      parseByteString(),
      parseByte<double>(),
      parseByte<double>(),
      parseByte<bool>(),
      parseByte<bool>(),
      parseByte<bool>(),
      parseByte<bool>())

  DEFAULT_SERIALIZE(string_to_byte(this->filepath),
                    type_to_byte(this->volume),
                    type_to_byte(this->pitch),
                    type_to_byte(this->loop),
                    type_to_byte(this->play),
                    type_to_byte(this->stop),
                    type_to_byte(this->playing))

  HOOKABLE(SoundEffect,
           HOOK(filepath),
           HOOK(volume),
           HOOK(pitch),
           HOOK(loop),
           HOOK(play),
           HOOK(stop),
           HOOK(playing))

  CHANGE_ENTITY_DEFAULT
};

inline Parser<SoundEffect> parseByteSoundEffect()
{
  return apply(
      [](std::string filepath,
         double volume,
         double pitch,
         bool loop,
         bool play,
         bool stop,
         bool playing)
      {
        return SoundEffect(
            std::move(filepath), volume, pitch, loop, play, stop, playing);
      },
      parseByteString(),
      parseByte<double>(),
      parseByte<double>(),
      parseByte<bool>(),
      parseByte<bool>(),
      parseByte<bool>(),
      parseByte<bool>());
}

class SoundManager
{
public:
  std::unordered_map<std::string, SoundEffect> _sound_effects;

  SoundManager() = default;

  SoundManager(std::unordered_map<std::string, SoundEffect> sound_effects)
      : _sound_effects(std::move(sound_effects))
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(
      SoundManager,
      [](std::unordered_map<std::string, SoundEffect> sound_effects)
      { return SoundManager(std::move(sound_effects)); },
      parseByteMap(parseByteString(), parseByteSoundEffect()))

  DEFAULT_SERIALIZE(map_to_byte<std::string, SoundEffect>(
      this->_sound_effects,
      std::function<ByteArray(std::string const&)>(string_to_byte),
      std::function<ByteArray(SoundEffect)>(
          [](const SoundEffect& sound_effect)
          { return sound_effect.to_bytes(); })))

  CHANGE_ENTITY_DEFAULT

  HOOKABLE(SoundManager)
};
