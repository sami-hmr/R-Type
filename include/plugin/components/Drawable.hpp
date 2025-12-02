/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Drawable
*/

#pragma once

#include "ParserTypes.hpp"
#include "ParserUtils.hpp"
#include "plugin/Byte.hpp"

struct Drawable
{
  Drawable() = default;
  EMPTY_BYTE_CONSTRUCTOR(Drawable)
  DEFAULT_SERIALIZE(ByteArray{})
};
