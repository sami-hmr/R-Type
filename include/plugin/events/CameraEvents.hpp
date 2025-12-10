#ifndef CAMERAEVENTS_HPP_
#define CAMERAEVENTS_HPP_

#include "EventMacros.hpp"
#include "ecs/Registry.hpp"


struct CamAggroEvent
{
    Registry::Entity target;

    CHANGE_ENTITY(result.target = map.at_second(target))
};

#endif /* !CAMERAEVENTS_HPP_ */
