#include "ATH.hpp"

#include "ecs/Registry.hpp"
#include "plugin/EntityLoader.hpp"

ATH::ATH(Registry &r, EntityLoader &l)
    : APlugin(r, l, {"position", "ui"}, {})
{
}