/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Background
*/

#ifndef BACKGROUND_HPP_
#define BACKGROUND_HPP_

#include <string>
#include <cstdint>
#include <utility>
#include "BaseTypes.hpp"
#include "ByteParser/ByteParser.hpp"
#include "TwoWayMap.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"


struct Background {
    
    enum RenderType : std::uint8_t {
            NOTHING,
            REPEAT,
            STRETCH
        };

    Background(std::vector<std::string> textures_path,
               RenderType render_type = RenderType::NOTHING,
               bool parallax = false)
        : textures_path(std::move(textures_path)),
          render_type(render_type),
          parallax(parallax)
    {}

    std::vector<std::string> textures_path;
    RenderType render_type;
    bool parallax = false;

    DEFAULT_BYTE_CONSTRUCTOR(Background, ([] (
        std::vector<std::vector<char>> textures_path,
        std::uint8_t render_type,
        bool parallax) {
            std::vector<std::string> paths;
            paths.reserve(textures_path.size());
            for (auto const& it : textures_path) {
                paths.emplace_back(it.begin(), it.end());
            }
            return Background(paths, static_cast<RenderType>(render_type), parallax);
        }),
        parseByteArray(parseByteArray(parseAnyChar())), parseByte<std::uint8_t>(), parseByte<bool>())
    DEFAULT_SERIALIZE(vector_to_byte<std::string>(this->textures_path, string_to_byte),
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
