#pragma once

#include "plugin/Hooks.hpp"
struct Damage
{
    Damage() = default;

    Damage(int amount)
        : amount(amount)
    {
    }

    int amount;

    HOOKABLE(Damage, HOOK(amount))
};
