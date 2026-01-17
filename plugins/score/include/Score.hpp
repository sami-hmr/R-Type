#pragma once

#include "plugin/APlugin.hpp"
#include "plugin/events/DeathEvent.hpp"


class Score : public APlugin
{
  public:
    Score(Registry& r, EventManager& em, EntityLoader& l);
  
    void init_score_manager(Registry::Entity& e, const JsonObject& obj);

    void on_death(Registry& r, const DeathEvent& event);
};