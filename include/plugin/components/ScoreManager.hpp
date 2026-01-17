#pragma once

#include "BaseTypes.hpp"
#include "ByteParser/ByteParser.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"

struct ScoreManager
{
    int score;
    int points_to_give;

    ScoreManager() = default;

    ScoreManager(int score, int points_to_give)
        : score(score), points_to_give(points_to_give)
    {
    }

    DEFAULT_BYTE_CONSTRUCTOR(ScoreManager,
                             ([](int score, int points_to_give)
                              { return ScoreManager(score, points_to_give); }),
                             parseByte<int>(),
                             parseByte<int>())

    DEFAULT_SERIALIZE(type_to_byte(this->score),
                      type_to_byte(this->points_to_give))


    CHANGE_ENTITY_DEFAULT

    HOOKABLE(ScoreManager, HOOK(score), HOOK(points_to_give))
};