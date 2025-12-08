#pragma once

#include <cstddef>
#include <mutex>
#include <queue>
#include <string>
#include "plugin/Byte.hpp"

struct ComponentBuilder {
    std::size_t entity;
    std::string id;
    ByteArray data;
};

struct EventBuilder {
    std::string event_id;
    ByteArray data;
};

template<typename T>
struct SharedQueue {
    std::mutex lock;
    std::queue<T> queue;
};
