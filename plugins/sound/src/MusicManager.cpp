

#include <stdexcept>
#include "plugin/components/MusicManager.hpp"

#include "Json/JsonParser.hpp"
#include "Sound.hpp"
#include "ecs/InitComponent.hpp"
#include "ecs/Registry.hpp"
#include "ecs/zipper/Zipper.hpp"
#include "ecs/zipper/ZipperIndex.hpp"
#include "plugin/events/MusicEvents.hpp"

void Sound::init_music_manager(Registry::Entity& e, const JsonObject& obj)
{
  std::unordered_map<std::string, SoundEffect> musics;

  JsonArray musics_array = get_value<MusicManager, JsonArray>(
                               this->_registry.get(), obj, e, "musics")
                               .value_or(JsonArray());

  try {
    for (const auto& music_value : musics_array) {
      JsonObject music_obj = std::get<JsonObject>(music_value.value);
      if (!music_obj.contains("name")) {
        std::cerr << "Error loading SoundEffect component: missing name in "
                     "JsonObject\n";
        return;
      }
      std::string name = get_value<SoundEffect, std::string>(
                             this->_registry.get(), music_obj, e, "name")
                             .value_or("");
      if (!music_obj.contains("filepath")) {
        std::cerr << "Error loading SoundEffect component: missing filepath in "
                     "JsonObject\n";
        return;
      }
      std::string filepath =
          get_value<SoundEffect, std::string>(
              this->_registry.get(), music_obj, e, "filepath")
              .value_or("");

      if (!music_obj.contains("volume")) {
        std::cerr << "Error loading SoundEffect component: missing volume in "
                     "JsonObject\n";
        return;
      }
      double volume = get_value<SoundEffect, double>(
                          this->_registry.get(), music_obj, e, "volume")
                          .value_or(100.0);

      double pitch = 1.0;
      if (music_obj.contains("pitch")) {
        pitch = get_value<SoundEffect, double>(
                    this->_registry.get(), music_obj, e, "pitch")
                    .value_or(1.0);
      }

      bool loop = false;
      if (music_obj.contains("loop")) {
        loop = get_value<SoundEffect, bool>(
                   this->_registry.get(), music_obj, e, "loop")
                   .value_or(false);
      }
      musics.insert_or_assign(name,
                              SoundEffect(filepath,
                                          volume,
                                          pitch,
                                          loop,
                                          /*play=*/false,
                                          /*stop=*/true,
                                          /*playing=*/false));
    }
  } catch (std::bad_variant_access const&) {
    std::cerr << "Error parsing SoundManager component: music_datas array "
                 "contains invalid value"
              << '\n';
    return;
  }
  init_component<MusicManager>(this->_registry.get(),
                               this->_event_manager.get(),
                               e,
                               MusicManager(std::move(musics)));
}

bool Sound::on_play_music(Registry& r, const PlayMusicEvent& event)
{
  for (const auto&& [e, music] : ZipperIndex<MusicManager>(r)) {
    if (e == event.entity) {
      if (!music.musics.contains(event.name)) {
        return false;
      }
      SoundEffect& music_data = music.musics.at(event.name);

      if (music_data.playing) {
        return false;
      }

      music_data.volume = event.volume;
      music_data.pitch = event.pitch;
      music_data.loop = event.loop;
      music_data.play = true;
      music_data.stop = false;
      return false;
    }
  }
  return false;
}

void Sound::music_system(Registry& r)
{
  for (auto&& [music] : Zipper<MusicManager>(r)) {
    for (auto& [name, music_data] : music.musics) {
      if (music_data.stop) {
        music_data.playing = false;
        music_data.stop = false;
      }
    }
  }
}
