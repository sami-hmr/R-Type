#include <cstddef>
#include <optional>

#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>

#include "SFMLRenderer.hpp"
#include "ecs/Registry.hpp"
#include "ecs/zipper/ZipperIndex.hpp"
#include "plugin/components/MusicManager.hpp"
#include "plugin/components/SoundManager.hpp"
#include "plugin/components/Volume.hpp"

sf::Music& SFMLRenderer::load_music(const std::string& path)
{
  if (_musics.contains(path)) {
    return _musics.at(path);
  }
  sf::Music music;
  if (!music.openFromFile(path)) {
    return this->_musics.at(placeholder);
  }
  _musics.insert_or_assign(path, std::move(music));
  return _musics.at(path);
}

sf::SoundBuffer& SFMLRenderer::load_sound(const std::string& sound)
{
  if (_sound_buffers.contains(sound)) {
    return _sound_buffers.at(sound);
  }
  sf::SoundBuffer sound_buffer;
  if (!sound_buffer.loadFromFile(sound)) {
    return _sound_buffers.at(placeholder);
  }
  _sound_buffers.insert_or_assign(sound, std::move(sound_buffer));
  return _sound_buffers.at(sound);
}

int SFMLRenderer::get_available_sound(sf::SoundBuffer& /*unused*/)
{
  for (std::size_t i = 0; i < this->_sounds.size(); ++i) {
    if (this->_sounds.at(i).first.has_value()
        && _sounds.at(i).first->getStatus() == sf::Sound::Status::Stopped)
    {
      return static_cast<int>(i);
    }
  }
  return -1;
}

void SFMLRenderer::sounds_system(Registry& r)
{
  for (auto&& [e, soundmanager] : ZipperIndex<SoundManager>(r)) {
    for (auto& [name, sound] : soundmanager._sound_effects) {
      auto vol = static_cast<float>((_sfx_volume / 100.0 * sound.volume)
                                    * (_master_volume / 100.0));
      for (auto& [sound_opt, sound_effect] : this->_sounds) {
        if (sound_opt && sound_effect.filepath == sound.filepath) {
          sound_opt->setVolume(vol);
          if (sound.stop) {
            sound_effect.playing = false;
            sound_effect.stop = false;
            sound.playing = false;
            sound.stop = false;
            sound_opt->stop();
          }
        }
      }
      sf::SoundBuffer& buffer = load_sound(sound.filepath);
      int sound_opt_idx = get_available_sound(buffer);
      if (sound_opt_idx == -1) {
        continue;
      }
      this->_sounds.at(sound_opt_idx).first->setBuffer(buffer);
      this->_sounds.at(sound_opt_idx)
          .first->setVolume(
              static_cast<float>((this->_sfx_volume / 100.0 * sound.volume)
                                 * (this->_master_volume / 100.0)));
      this->_sounds.at(sound_opt_idx)
          .first->setPitch(static_cast<float>(sound.pitch));
      this->_sounds.at(sound_opt_idx).first->setLooping(sound.loop);

      if (sound.play && !sound.playing) {
        sound.playing = true;
        sound.play = false;
        this->_sounds.at(sound_opt_idx).second = sound;
        this->_sounds.at(sound_opt_idx).first->play();
      }
    }
  }
}

void SFMLRenderer::musics_system(Registry& r)
{
  for (auto&& [e, musicmanager] : ZipperIndex<MusicManager>(r)) {
    for (auto& [name, music] : musicmanager.musics) {
      sf::Music& buffer = load_music(music.filepath);

      buffer.setVolume(
          static_cast<float>((this->_music_volume / 100.0 * music.volume)
                             * (this->_master_volume / 100.0)));
      buffer.setPitch(static_cast<float>(music.pitch));
      buffer.setLooping(music.loop);
      if (music.play && !music.playing) {
        music.playing = true;
        music.play = false;
        buffer.play();
        std::cout << "playing music sfml\n";
      }
      if ((music.stop && music.playing)
          || buffer.getStatus() == sf::Music::Status::Stopped)
      {
        buffer.stop();
        music.playing = false;
        music.stop = false;
      }
    }
  }
}

void SFMLRenderer::volumes_system(Registry& r)
{
  for (auto&& [s, master_volume] : Zipper<Scene, MasterVolume>(r)) {
    this->_master_volume = master_volume.value;
  }
  for (auto&& [s, sfx_volume] : Zipper<Scene, SFXVolume>(r)) {
    this->_sfx_volume = sfx_volume.value;
  }
  for (auto&& [s, music_volume] : Zipper<Scene, MusicVolume>(r)) {
    this->_music_volume = music_volume.value;
  }
}
