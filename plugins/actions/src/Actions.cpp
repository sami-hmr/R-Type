#include "Actions.hpp"

#include "Json/JsonParser.hpp"
#include "TwoWayMap.hpp"
#include "plugin/components/ActionTrigger.hpp"

static const TwoWayMap<std::string, TriggerType> TRIGGER_TYPES = {
    {"click", TriggerType::CLICK},
    {"hover", TriggerType::HOVER},
    {"collision", TriggerType::COLLISION},
    {"key_press", TriggerType::KEY_PRESS},
    {"timer", TriggerType::TIMER},
};

static const TwoWayMap<std::string, ActionType> ACTION_TYPES = {
    {"emit_event", ActionType::EMIT_EVENT},
    {"delay", ActionType::DELAY},
    {"kill_entity", ActionType::KILL_ENTITY},
    {"spawn_entity", ActionType::SPAWN_ENTITY},
    {"modify_component", ActionType::MODIFY_COMPONENT}};

static const std::unordered_map<std::string, Key> KEY_MAPPING = {
    {"ENTER", Key::ENTER},
    {"R", Key::R},
    {"Z", Key::Z},
    {"Q", Key::Q},
    {"S", Key::S},
    {"D", Key::D},
    {"SPACE", Key::SPACE},
    {"ECHAP", Key::ECHAP},
    {"DELETE", Key::DELETE}};

Actions::Actions(Registery& r, EntityLoader& l)
    : APlugin(r,
              l,
              {},
              {{"ActionTrigger",
                [this](size_t entity, const JsonVariant& config)
                {
                  try {
                    JsonObject obj = std::get<JsonObject>(config);
                    this->init_action_trigger(entity, obj);
                  } catch (std::bad_variant_access const&) {
                    std::cout << "Error initializing component \"ActionTrigger\": "
                                 "only JsonObjects are supported\n";
                    return;
                  }
                }}})
{
  _registery.get().register_component<ActionTrigger>();

  _registery.get().add_system<ActionTrigger>(
      [this](Registery&, SparseArray<ActionTrigger> const&)
      { this->process_triggers(); },
      5);

  _registery.get().on<CollisionEvent>([this](const CollisionEvent& event)
                                      { this->on_collision(event); });

  _registery.get().on<KeyPressedEvent>([this](const KeyPressedEvent& event)
                                       { this->on_key_pressed(event); });
}

void Actions::init_action_trigger(Registery::Entity entity,
                                  JsonObject const& obj)
{
  TriggerType trigger_type = TriggerType::CLICK;
  std::vector<Action> actions;
  std::string condition;

  if (obj.contains("trigger")) {
    auto trigger_str = std::get<std::string>(obj.at("trigger").value);
    try {
      trigger_type = TRIGGER_TYPES.at(trigger_str);
    } catch (const std::out_of_range&) {
    }
  }

  if (obj.contains("condition")) {
    condition = std::get<std::string>(obj.at("condition").value);
  }

  if (obj.contains("actions")) {
    auto actions_array = std::get<JsonArray>(obj.at("actions").value);
    for (auto const& action_val : actions_array) {
      auto action_obj = std::get<JsonObject>(action_val.value);

      ActionType type = ActionType::EMIT_EVENT;
      std::string event_name;
      JsonObject params;
      double delay = 0.0;

      if (action_obj.contains("type")) {
        auto type_str = std::get<std::string>(action_obj.at("type").value);
        try {
          type = ACTION_TYPES.at(type_str);
        } catch (const std::out_of_range&) {
        }
      }

      if (action_obj.contains("event")) {
        event_name = std::get<std::string>(action_obj.at("event").value);
      }

      if (action_obj.contains("params")
          && std::holds_alternative<JsonObject>(action_obj.at("params").value))
      {
        params = std::get<JsonObject>(action_obj.at("params").value);
      }

      if (action_obj.contains("data")) {
        params["data"] = action_obj.at("data");
      }

      if (action_obj.contains("delay")) {
        delay = std::get<double>(action_obj.at("delay").value);
      }

      actions.emplace_back(type, event_name, params, delay);
    }
  }

  _registery.get().emplace_component<ActionTrigger>(
      entity, ActionTrigger(trigger_type, actions, condition));
}

void Actions::process_triggers()
{
  auto& triggers = _registery.get().get_components<ActionTrigger>();

  for (size_t entity = 0; entity < triggers.size(); ++entity) {
    if (!triggers[entity].has_value()) {
      continue;
    }
    if (_registery.get().is_entity_dying(entity)) {
      continue;
    }

    auto& trigger = triggers[entity].value();

    if (trigger.trigger == TriggerType::TIMER) {
      auto now = std::chrono::steady_clock::now();
      auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                         now - trigger.last_trigger_time)
                         .count()
          / 1000.0;

      if (!trigger.actions.empty()
          && elapsed >= trigger.actions[0].delay_seconds)
      {
        for (auto const& action : trigger.actions) {
          execute_action(entity, action);
        }
        trigger.last_trigger_time = now;
      }
    }
  }
}

void Actions::execute_action(Registery::Entity entity, const Action& action)
{
  switch (action.type) {
    case ActionType::EMIT_EVENT:
      this->_registery.get().emit(action.event_name, action.params);
      break;

    case ActionType::KILL_ENTITY:
      _registery.get().kill_entity(entity);
      break;

    case ActionType::DELAY:
    case ActionType::SPAWN_ENTITY:
    case ActionType::MODIFY_COMPONENT:
      break;
  }
}

void Actions::on_collision(const CollisionEvent& event)
{
  auto& triggers = _registery.get().get_components<ActionTrigger>();

  if (event.a < triggers.size() && triggers[event.a].has_value()) {
    auto& trigger = triggers[event.a].value();
    if (trigger.trigger == TriggerType::COLLISION && !trigger.triggered) {
      for (auto const& action : trigger.actions) {
        execute_action(event.a, action);
      }
      trigger.triggered = true;
    }
  }
}

void Actions::on_key_pressed(const KeyPressedEvent& event)
{
  auto& triggers = _registery.get().get_components<ActionTrigger>();

  for (size_t entity = 0; entity < triggers.size(); ++entity) {
    if (!triggers[entity].has_value()) {
      continue;
    }

    auto& trigger = triggers[entity].value();
    if (trigger.trigger == TriggerType::KEY_PRESS) {
      bool should_trigger = false;

      if (KEY_MAPPING.contains(trigger.condition)) {
        Key key = KEY_MAPPING.at(trigger.condition);
        if (event.key_pressed.contains(key)) {
          should_trigger = event.key_pressed.at(key);
        }
      }

      if (should_trigger) {
        for (auto const& action : trigger.actions) {
          execute_action(entity, action);
        }
      }
    }
  }
}

extern "C"
{
void* entry_point(Registery& r, EntityLoader& e)
{
  return new Actions(r, e);
}
}
