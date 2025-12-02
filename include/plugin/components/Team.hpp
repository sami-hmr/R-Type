#pragma once


#include <string>

struct Team {
    Team() = default;

    Team(std::string name)
        : name(std::move(name))
    {
    }
    std::string name;
};
