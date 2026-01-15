#include <optional>

#include "plugin/components/Sound.hpp"

#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>

#include "SFMLRenderer.hpp"
#include "ecs/Registry.hpp"
#include "ecs/zipper/ZipperIndex.hpp"

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
      sound_opt->get().setVolume(static_cast<float>(sound.volume));
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
