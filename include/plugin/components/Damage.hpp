#pragma once

struct Damage
{
    Damage() = default;
    
    Damage(int amount)
        : amount(amount)
    {
    }

    int amount;
};
