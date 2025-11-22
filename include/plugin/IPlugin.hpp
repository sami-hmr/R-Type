#pragma once

#include "ecs/Registery.hpp"
#include "Json/JsonParser.hpp"
#include <string>

class IPlugin {
    public:
    virtual ~IPlugin() = default;

    virtual void setComponent(Registery::Entity entity,
        std::string const &key, JsonVariant const&) = 0;
};
