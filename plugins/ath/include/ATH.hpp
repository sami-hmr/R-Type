#pragma once

#include "ecs/Registry.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"

class ATH : public APlugin {

    ATH(Registry &r, EntityLoader &l);
};