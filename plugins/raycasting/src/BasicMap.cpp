#include "plugin/components/BasicMap.hpp"

#include "Json/JsonParser.hpp"
#include "Raycasting.hpp"
#include "ecs/InitComponent.hpp"
#include "ecs/Registry.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/components/Collidable.hpp"
#include "plugin/components/Drawable.hpp"
#include "plugin/components/Position.hpp"
#include "plugin/events/EntityManagementEvent.hpp"

void Raycasting::init_basic_map(Ecs::Entity& e, const JsonObject& obj)
{
  Vector2D size;
  if (!obj.contains("size")) {
    std::cerr << "[Raycasting] BasicMap component missing 'size' field"
              << std::endl;
    return;
  }
  size = get_value<BasicMap, Vector2D>(
             this->_registry.get(), obj, e, "size", "width", "height")
             .value();

  std::vector<std::vector<int>> data;
  if (!obj.contains("data")) {
    std::cerr << "[Raycasting] BasicMap component missing 'data' field"
              << std::endl;
    return;
  }
  JsonArray lines =
      get_value<BasicMap, JsonArray>(this->_registry.get(), obj, e, "data")
          .value();

  for (const auto& line : lines) {
    try {
      JsonArray col = std::get<JsonArray>(line.value);
      data.emplace_back();
      for (const auto& nb : col) {
        int nbr = std::get<int>(nb.value);
        data.back().push_back(nbr);
      }
    } catch (std::bad_variant_access& e) {
      std::cerr << "error parsing basic map, bad access tu connais"
                << std::endl;
      return;
    }
  }
  std::unordered_map<int, std::unordered_map<std::string, TileData>> tiles_data;
  std::unordered_map<int, bool> collidable_tiles;

  if (obj.contains("tiles_data")) {
    JsonObject tiles_data_obj = get_value<BasicMap, JsonObject>(
                                    this->_registry.get(), obj, e, "tiles_data")
                                    .value();

    std::cout << "Parsing tiles_data\n";
    for (const auto& [tile, tiles_obj] : tiles_data_obj) {
      std::cout << "Parsing tile " << tile << "\n";
      JsonObject tile_obj = std::get<JsonObject>(tiles_obj.value);
      int tile_key = std::stoi(tile);
      bool collidable = true;
      if (tile_obj.contains("collidable")) {
        collidable = get_value<BasicMap, bool>(
                         this->_registry.get(), tile_obj, e, "collidable")
                         .value();
      }
      collidable_tiles.insert_or_assign(tile_key, collidable);
      JsonObject textures_obj = std::get<JsonObject>(
          get_value<BasicMap, JsonValue>(
              this->_registry.get(), tile_obj, e, "textures")
              .value()
              .value);
      for (const auto& [direction, tile_data_val] : textures_obj) {
        JsonObject tile_data_obj = std::get<JsonObject>(tile_data_val.value);
        Vector2D tile_size =
            get_value<BasicMap, Vector2D>(this->_registry.get(),
                                          tile_data_obj,
                                          e,
                                          "size",
                                          "width",
                                          "height")
                .value();
        Vector2D tile_pos =
            get_value<BasicMap, Vector2D>(
                this->_registry.get(), tile_data_obj, e, "pos", "x", "y")
                .value();
        std::string texture_path =
            get_value<BasicMap, std::string>(
                this->_registry.get(), tile_data_obj, e, "texture_path")
                .value();
        tiles_data[tile_key].insert_or_assign(
            direction, TileData(tile_size, tile_pos, texture_path, collidable));
      }
    }
  }

  TileData floor_data;
  if (obj.contains("floor_data")) {
    JsonObject floor_obj = get_value<BasicMap, JsonObject>(
                               this->_registry.get(), obj, e, "floor_data")
                               .value();
    floor_data.size =
        get_value<BasicMap, Vector2D>(
            this->_registry.get(), floor_obj, e, "size", "width", "height")
            .value();
    floor_data.pos = get_value<BasicMap, Vector2D>(
                         this->_registry.get(), floor_obj, e, "pos", "x", "y")
                         .value();
    floor_data.texture_path =
        get_value<BasicMap, std::string>(
            this->_registry.get(), floor_obj, e, "texture_path")
            .value();
  }

  TileData ceiling_data;
  if (obj.contains("ceiling_data")) {
    JsonObject ceiling_obj = get_value<BasicMap, JsonObject>(
                                 this->_registry.get(), obj, e, "ceiling_data")
                                 .value();
    ceiling_data.size =
        get_value<BasicMap, Vector2D>(
            this->_registry.get(), ceiling_obj, e, "size", "width", "height")
            .value();
    ceiling_data.pos =
        get_value<BasicMap, Vector2D>(
            this->_registry.get(), ceiling_obj, e, "pos", "x", "y")
            .value();
    ceiling_data.texture_path =
        get_value<BasicMap, std::string>(
            this->_registry.get(), ceiling_obj, e, "texture_path")
            .value();
  }

  init_component<BasicMap>(this->_registry.get(),
                           this->_event_manager.get(),
                           e,
                           size,
                           data,
                           tiles_data,
                           floor_data,
                           ceiling_data);
}
