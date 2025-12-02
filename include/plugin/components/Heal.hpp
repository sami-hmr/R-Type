#pragma once

struct Heal
{
    Heal() = default;
    
    Heal(int amount)
        : amount(amount)
    {
    }

    int amount;
};
