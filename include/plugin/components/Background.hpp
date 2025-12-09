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
#include "TwoWayMap.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/Byte.hpp"
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
           double posx,
           double posy)
      : active(active)
      , speed(x, y)
      , framerate(framerate)
  {
  }

  Parallax(bool active, Vector2D speed, double framerate)
      : active(active)
      , speed(speed)
      , framerate(framerate)
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
          double framerate,
          double posx,
          double posy)
       { return Parallax(active, x, y, framerate, posx, posy); }),
      parseByte<bool>(),
      parseByte<double>(),
      parseByte<double>(),
      parseByte<double>(),
      parseByte<double>(),
      parseByte<double>())
  DEFAULT_SERIALIZE(type_to_byte(this->active),
                    type_to_byte(this->speed.x),
                    type_to_byte(this->speed.y),
                    type_to_byte(this->framerate),
                    type_to_byte(this->pos.x),
                    type_to_byte(this->pos.y))
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
             Parallax parallax)
      : textures_path(std::move(textures_path))
      , render_type(render_type)
      , parallax(parallax)
  {
  }

  std::vector<std::string> textures_path;
  RenderType render_type;
  Parallax parallax;

  DEFAULT_BYTE_CONSTRUCTOR(Background,
                           (
                               [](std::vector<std::vector<char>> textures_path,
                                  std::uint8_t render_type,
                                  Parallax parallax)
                               {
                                 std::vector<std::string> paths;
                                 paths.reserve(textures_path.size());
                                 for (auto const& it : textures_path) {
                                   paths.emplace_back(it.begin(), it.end());
                                 }
                                 return Background(
                                     paths,
                                     static_cast<RenderType>(render_type),
                                     parallax);
                               }),
                           parseByteArray(parseByteArray(parseAnyChar())),
                           parseByte<std::uint8_t>(),
                           parseByte<Parallax>())
  DEFAULT_SERIALIZE(vector_to_byte<std::string>(this->textures_path,
                                                string_to_byte),
                    type_to_byte(static_cast<std::uint8_t>(this->render_type)),
                    type_to_byte(this->parallax))
  HOOKABLE(Background, HOOK(textures_path), HOOK(render_type), HOOK(parallax));
};

static const std::map<std::string, Background::RenderType> render_type_map = {
    {"NOTHING", Background::RenderType::NOTHING},
    {"REPEAT", Background::RenderType::REPEAT},
    {"STRETCH", Background::RenderType::STRETCH},
};

#endif /* !BACKGROUND_HPP_ */
