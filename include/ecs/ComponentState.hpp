#pragma once

#include <vector>
#include "plugin/Byte.hpp"

struct ComponentState {
    std::string id;
    std::vector<std::pair<size_t, ByteArray>> comps;
};
