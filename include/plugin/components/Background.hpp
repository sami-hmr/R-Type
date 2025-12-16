/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Background
*/

#ifndef BACKGROUND_HPP_
#define BACKGROUND_HPP_

#include <cstdint>
#include <string>
#include <utility>

#include "BaseTypes.hpp"
#include "ByteParser/ByteParser.hpp"
#include "plugin/Byte.hpp"
#include "TwoWayMap.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/Hooks.hpp"

struct Parallax
{
  Parallax()
      : active(false)
      , speed(0, 0)
      , framerate(0.0)
  {
  }

  Parallax(bool active,
           double x,
           double y,
           double framerate,
          Vector2D pos = {0, 0})
      : active(active)
      , speed(x, y)
      , framerate(framerate)
      , pos(pos)
  {
  }

  Parallax(bool active, Vector2D speed, double framerate, Vector2D pos = {0, 0})
      : active(active)
      , speed(speed)
      , framerate(framerate)
      , pos(pos)
  {
  }

  bool active = false;
  Vector2D speed;
  Vector2D pos = {0, 0};
  double framerate;

  DEFAULT_BYTE_CONSTRUCTOR(
      Parallax,
      ([](bool active,
          double x,
          double y,
          double framerate, Vector2D pos)
       { return Parallax(active, x, y, framerate, pos); }),
      parseByte<bool>(),
      parseByte<double>(),
      parseByte<double>(),
      parseByte<double>(),
    parseVector2D())
  DEFAULT_SERIALIZE(type_to_byte(this->active),
                    type_to_byte(this->speed.x),
                    type_to_byte(this->speed.y),
                    type_to_byte(this->framerate),
                    vector2DToByte(this->pos))
  HOOKABLE(Parallax, HOOK(active), HOOK(speed), HOOK(framerate), HOOK(pos));
};

struct Background
{
  enum RenderType : std::uint8_t
  {
    NOTHING,
    REPEAT,
    STRETCH
  };

  Background(std::vector<std::string> textures_path,
             RenderType render_type,
             Parallax parallax,
             Vector2D position = {0, 0})
      : textures_path(std::move(textures_path))
      , render_type(render_type)
      , parallax(parallax)
      , position(position)
  {
  }

  std::vector<std::string> textures_path;
  RenderType render_type;
  Parallax parallax;
  Vector2D position = {0, 0};

  CHANGE_ENTITY_DEFAULT

  DEFAULT_BYTE_CONSTRUCTOR(Background,
                           (
                               [](std::vector<std::vector<char>> textures_path,
                                  std::uint8_t render_type,
                                  Parallax parallax, Vector2D position)
                               {
                                 std::vector<std::string> paths;
                                 paths.reserve(textures_path.size());
                                 for (auto const& it : textures_path) {
                                   paths.emplace_back(it.begin(), it.end());
                                 }
                                 return Background(
                                     paths,
                                     static_cast<RenderType>(render_type),
                                     parallax, position);
                               }),
                           parseByteArray(parseByteArray(parseAnyChar())),
                           parseByte<std::uint8_t>(),
                           parseByte<Parallax>(), parseVector2D())
  DEFAULT_SERIALIZE(vector_to_byte<std::string>(this->textures_path,
                                                string_to_byte),
                    type_to_byte(static_cast<std::uint8_t>(this->render_type)),
                    parallax.to_bytes(),
                    vector2DToByte(this->position))
  HOOKABLE(Background, HOOK(textures_path), HOOK(render_type), HOOK(parallax), HOOK(position));
};

static const std::map<std::string, Background::RenderType> render_type_map = {
    {"NOTHING", Background::RenderType::NOTHING},
    {"REPEAT", Background::RenderType::REPEAT},
    {"STRETCH", Background::RenderType::STRETCH},
};

#endif /* !BACKGROUND_HPP_ */
