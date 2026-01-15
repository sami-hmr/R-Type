#pragma once

#include "SoundManager.hpp"


class MusicManager
{
public:
  std::unordered_map<std::string, SoundEffect> musics;

  MusicManager() = default;

  MusicManager(std::unordered_map<std::string, SoundEffect> sound_effects)
      : musics(std::move(sound_effects))
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(
      MusicManager,
      [](std::unordered_map<std::string, SoundEffect> sound_effects)
      { return MusicManager(std::move(sound_effects)); },
      parseByteMap(parseByteString(), parseByteSoundEffect()))

  DEFAULT_SERIALIZE(map_to_byte<std::string, SoundEffect>(
      this->musics,
      std::function<ByteArray(std::string const&)>(string_to_byte),
      std::function<ByteArray(SoundEffect)>(
          [](const SoundEffect& sound_effect)
          { return sound_effect.to_bytes(); })))

  CHANGE_ENTITY_DEFAULT

  HOOKABLE(MusicManager)
};