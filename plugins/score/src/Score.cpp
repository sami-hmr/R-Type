#include "Score.hpp"

#include "Json/JsonParser.hpp"
#include "ecs/InitComponent.hpp"
#include "ecs/Registry.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/IPlugin.hpp"
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

void Score::on_death(Registry& r, const DeathEvent& event)
{

  if (!r.has_component<ScoreManager>(event.killer))
  {
    if (event.killer == 3) {
      std::cout << "killer has no ScoreManager component, no score awarded." << std::endl;
    }
    return;
  }
  if (!r.has_component<ScoreManager>(event.entity)) {
    if (event.killer == 3) {
      std::cout << "entity has no ScoreManager component, no score to give." << std::endl;
    }
    return;
  }

  std::cout << "Entity " << event.entity << " was killed by " << event.killer
            << std::endl;
  auto& scores = r.get_components<ScoreManager>();
  if (event.killer < scores.size() && scores[event.killer].has_value()
      && event.entity < scores.size() && scores[event.entity].has_value())
  {
    scores[event.killer]->score += scores[event.entity]->points_to_give;

    this->_event_manager.get().emit<ComponentBuilder>(
        event.killer,
        r.get_component_key<ScoreManager>(),
        scores[event.killer]->to_bytes());
  }
  std::cout << "Entity " << event.killer << " scored! New score: "
            << scores[event.killer]->score << std::endl;
}

void Score::init_score_manager(Ecs::Entity& e, const JsonObject& obj)
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

extern "C"
{
PLUGIN_EXPORT void* entry_point(Registry& r, EventManager& em, EntityLoader& l)
{
  return new Score(r, em, l);
}
}
