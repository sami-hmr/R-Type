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

  Parallax(bool active, double x, double y, double framerate)
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
      ([](bool active, double x, double y, double framerate)
       { return Parallax(active, x, y, framerate); }),
      parseByte<bool>(),
      parseByte<double>(),
      parseByte<double>(),
      parseByte<double>())
  DEFAULT_SERIALIZE(type_to_byte(this->active),
                    type_to_byte(this->speed.x),
                    type_to_byte(this->speed.y),
                    type_to_byte(this->framerate))
  HOOKABLE(Parallax, HOOK(active), HOOK(speed), HOOK(framerate), HOOK(pos));
};

inline Parser<Parallax> parse_parallax()
{
  return apply([](bool active, double x, double y, double framerate)
               { return Parallax(active, x, y, framerate); },
               parseByte<bool>(),
               parseByte<double>(),
               parseByte<double>(),
               parseByte<double>());
}

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

  CHANGE_ENTITY_DEFAULT

  DEFAULT_BYTE_CONSTRUCTOR(Background,
                           (
                               [](std::vector<std::string> textures_path,
                                  std::uint8_t render_type,
                                  Parallax parallax)
                               {
                                 return Background(
                                     std::move(textures_path),
                                     static_cast<RenderType>(render_type),
                                     parallax);
                               }),
                           parseByteArray(parseByteString()),
                           parseByte<std::uint8_t>(),
                           parse_parallax())
  DEFAULT_SERIALIZE(vector_to_byte<std::string>(
                        this->textures_path,
                        SERIALIZE_FUNCTION<std::string>(string_to_byte)),
                    type_to_byte<std::uint8_t>(this->render_type),
                    parallax.to_bytes())
  HOOKABLE(Background, HOOK(textures_path), HOOK(render_type), HOOK(parallax));
};

static const std::map<std::string, Background::RenderType> render_type_map = {
    {"NOTHING", Background::RenderType::NOTHING},
    {"REPEAT", Background::RenderType::REPEAT},
    {"STRETCH", Background::RenderType::STRETCH},
};

#endif /* !BACKGROUND_HPP_ */
