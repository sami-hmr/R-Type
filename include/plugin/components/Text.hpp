/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Text
*/

#pragma once
#include <string>
#include <vector>
#include "ByteParser/ByteParser.hpp"
#include "plugin/Byte.hpp"

struct Text
{
    Text(std::string font_path)
        : font_path(std::move(font_path))
    {
    }

    DEFAULT_BYTE_CONSTRUCTOR(Text,
                             ([](std::vector<char> font_path)
                             {
                               return Text(std::string(font_path.begin(), font_path.end()));
                             }),
                             parseByteArray(parseAnyChar()))
    DEFAULT_SERIALIZE(string_to_byte(this->font_path))

  std::string font_path;
};
