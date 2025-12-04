#pragma once

#include "plugin/Hooks.hpp"
struct Heal
{
    Heal() = default;

    Heal(int amount)
        : amount(amount)
    {
    }

    int amount;

    HOOKABLE(Heal, HOOK(amount))
};
