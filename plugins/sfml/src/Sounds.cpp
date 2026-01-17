#include <optional>

#include "plugin/components/SoundManager.hpp"
#include "plugin/components/MusicManager.hpp"

#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>

#include "SFMLRenderer.hpp"
#include "ecs/Registry.hpp"
#include "ecs/zipper/ZipperIndex.hpp"
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

std::optional<std::reference_wrapper<sf::Sound>>
SFMLRenderer::get_available_sound(sf::SoundBuffer& buffer)
{
  for (auto& sound : this->_sounds) {
    if (sound.has_value() && sound->getStatus() == sf::Sound::Status::Stopped) {
      return std::ref(sound.value());
    }
  }
  return std::nullopt;
}



void SFMLRenderer::sounds_system(Registry& r)
{
  for (auto&& [e, soundmanager] : ZipperIndex<SoundManager>(r)) {
    for (auto& [name, sound] : soundmanager._sound_effects) {
      sf::SoundBuffer& buffer = load_sound(sound.filepath);
      std::optional<std::reference_wrapper<sf::Sound>> sound_opt =
          get_available_sound(buffer);
      if (!sound_opt.has_value()) {
        continue;
      }
      sound_opt->get().setBuffer(buffer);
      sound_opt->get().setVolume(static_cast<float>((this->_sfx_volume / 100.0 * sound.volume) * (this->_master_volume / 100.0)));
      sound_opt->get().setPitch(static_cast<float>(sound.pitch));
      sound_opt->get().setLooping(sound.loop);

      if (sound.play && !sound.playing) {
        sound.playing = true;
        sound.play = false;
        sound_opt->get().play();
      }
      if ((sound.stop && sound.playing)
          || sound_opt->get().getStatus() == sf::Sound::Status::Stopped)
      {
        sound_opt->get().stop();
        sound.playing = false;
        sound.stop = false;
      }
    }
  }
}

void SFMLRenderer::musics_system(Registry& r)
{
  for (auto&& [e, musicmanager] : ZipperIndex<MusicManager>(r)) {
    for (auto& [name, music] : musicmanager.musics) {
      sf::Music &buffer = load_music(music.filepath);

      buffer.setVolume(static_cast<float>((this->_music_volume / 100.0 * music.volume) * (this->_master_volume / 100.0)));
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