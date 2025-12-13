#pragma once

#include <string>
#include "libs/Vector2D.hpp"

struct Bar {
    std::string texture_path;
    double current_value;
    double max_value;
    Vector2D size;
}; 