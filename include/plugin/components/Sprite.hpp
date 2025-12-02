/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Sprite
*/

#pragma once

#include <string>

#include "ByteParser/ByteParser.hpp"
#include "plugin/Byte.hpp"

struct Sprite
{
  Sprite(std::string texture_path)
      : texture_path(std::move(texture_path))
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(Sprite,
                           ([](std::vector<char> texture_path)
                           {
                             return Sprite(std::string(texture_path.begin(), texture_path.end()));
                           }),
                           parseByteArray(parseAnyChar()))
  DEFAULT_SERIALIZE(string_to_byte(this->texture_path))

  std::string texture_path;
};
