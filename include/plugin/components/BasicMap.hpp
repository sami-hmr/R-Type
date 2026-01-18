#pragma once

#include <vector>

#include "ByteParser/ByteParser.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/Byte.hpp"
#include "plugin/HookMacros.hpp"
#include "plugin/events/EventMacros.hpp"

struct TileData
{
  Vector2D size;
  Vector2D pos;
  std::string texture_path;
  bool collidable = true;

  TileData() = default;

  TileData(Vector2D size,
           Vector2D pos,
           std::string texture_path,
           bool collidable = true)
      : size(size)
      , pos(pos)
      , texture_path(std::move(texture_path))
      , collidable(collidable) {};

  DEFAULT_BYTE_CONSTRUCTOR(
      TileData,
      [](Vector2D size, Vector2D pos, std::string texture_path, bool collidable)
      { return TileData(size, pos, std::move(texture_path), collidable); },
      parseVector2D(),
      parseVector2D(),
      parseByteString(),
      parseByte<bool>())

  DEFAULT_SERIALIZE(vector2DToByte(this->size),
                    vector2DToByte(this->pos),
                    string_to_byte(this->texture_path),
                    type_to_byte<bool>(this->collidable))
  HOOKABLE(
      TileData, HOOK(size), HOOK(pos), HOOK(texture_path), HOOK(collidable))

  CHANGE_ENTITY_DEFAULT
};

inline Parser<TileData> parse_tile_data()
{
  return apply(
      [](Vector2D size, Vector2D pos, std::string texture_path, bool collidable)
      { return TileData(size, pos, texture_path, collidable); },
      parseVector2D(),
      parseVector2D(),
      parseByteString(),
      parseByte<bool>());
}

struct BasicMap
{
  Vector2D size;
  std::vector<std::vector<int>> data;
  std::unordered_map<int, std::unordered_map<std::string, TileData>> tiles_data;
  TileData floor_data;
  TileData ceiling_data;

  BasicMap() = default;

  BasicMap(Vector2D size,
           std::vector<std::vector<int>> data,
           std::unordered_map<int, std::unordered_map<std::string, TileData>>
               tiles_data,
           TileData floor_data = TileData(),
           TileData ceiling_data = TileData())
      : size(size)
      , data(std::move(data))
      , tiles_data(std::move(tiles_data))
      , floor_data(std::move(floor_data))
      , ceiling_data(std::move(ceiling_data))
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(
      BasicMap,
      [](Vector2D size,
         std::vector<std::vector<int>> data,
         std::unordered_map<int, std::unordered_map<std::string, TileData>>
             tiles_data,
         TileData floor_data,
         TileData ceiling_data)
      {
        return BasicMap(size,
                        std::move(data),
                        std::move(tiles_data),
                        std::move(floor_data),
                        std::move(ceiling_data));
      },
      parseVector2D(),
      parseByteArray(parseByteArray(parseByte<int>())),
      parseByteMap(parseByte<int>(),
                   parseByteMap(parseByteString(), parse_tile_data())),
      parse_tile_data(),
      parse_tile_data())

  DEFAULT_SERIALIZE(
      vector2DToByte(this->size),
      vector_to_byte(this->data,
                     SERIALIZE_FUNCTION<std::vector<int>>(vector_to_byte<int>,
                                                          TTB_FUNCTION<int>())),
      map_to_byte(this->tiles_data,
                  TTB_FUNCTION<int>(),
                  SERIALIZE_FUNCTION<std::unordered_map<std::string, TileData>>(
                      map_to_byte<std::string, TileData>,
                      string_to_byte,
                      SERIALIZE_FUNCTION<TileData>([](const TileData& td)
                                                   { return td.to_bytes(); }))),
      this->floor_data.to_bytes(),
      this->ceiling_data.to_bytes())

  CHANGE_ENTITY_DEFAULT

  HOOKABLE(
      BasicMap, HOOK(size), HOOK(data), HOOK(floor_data), HOOK(ceiling_data))
};
