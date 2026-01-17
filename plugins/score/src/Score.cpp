#include "Score.hpp"

#include "Json/JsonParser.hpp"
#include "ecs/InitComponent.hpp"
#include "ecs/Registry.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/components/ScoreManager.hpp"
#include "plugin/events/DeathEvent.hpp"

Score::Score(Registry& r, EventManager& em, EntityLoader& l)
    : APlugin("score",
              r,
              em,
              l,
              {"life"},
              {COMP_INIT(ScoreManager, ScoreManager, init_score_manager)})
{
  REGISTER_COMPONENT(ScoreManager)
  SUBSCRIBE_EVENT(DeathEvent, { this->on_death(this->_registry.get(), event); })
}

void Score::init_score_manager(Registry::Entity& e, const JsonObject& obj)
{
  int initial_score =
      get_value<ScoreManager, int>(this->_registry.get(), obj, e, "score")
          .value_or(0);
  std::optional<int> points_to_give = get_value<ScoreManager, int>(
      this->_registry.get(), obj, e, "points_to_give");
  if (!points_to_give.has_value()) {
    std::cerr << "Warning: points_to_give not set for ScoreManager, defaulting "
                 "to 0\n";
    points_to_give = 0;
  }
  init_component<ScoreManager>(this->_registry.get(),
                               this->_event_manager.get(),
                               e,
                               initial_score,
                               points_to_give.value());
}
