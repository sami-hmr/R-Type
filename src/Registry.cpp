

#include <optional>
#include <vector>

#include "ecs/Registry.hpp"

#include "ecs/Systems.hpp"

Registry::Entity Registry::spawn_entity()
{
  Entity to_return = 0;
  if (this->_dead_entities.empty()) {
    to_return = this->_max;
    this->_max += 1;
  } else {
    to_return = this->_dead_entities.front();
    this->_dead_entities.pop();
  }
  return to_return;
}

void Registry::kill_entity(Entity const& e)
{
  _entities_to_kill.insert(e);
}

bool Registry::is_entity_dying(Entity const& e) const
{
  return _entities_to_kill.contains(e);
}

void Registry::process_entity_deletions()
{
  for (auto const& e : _entities_to_kill) {
    for (auto const& [_, f] : this->_delete_functions) {
      f(e);
    }
    this->_dead_entities.push(e);
  }
  _entities_to_kill.clear();
}

void Registry::emplace_component(Entity const& to,
                                 std::string const& string_id,
                                 ByteArray const& bytes)
{
  try {
    this->_emplace_functions.at(this->_index_getter.at_second(string_id))(
        to, bytes);
  } catch (std::out_of_range const&) {
    std::cerr << "error: unknow component :" << string_id << "\n";
  }
}

void Registry::run_systems()
{
  update_bindings();

  std::vector<System<>> pending = this->_frequent_systems;
  for (auto const& f : pending) {
    f();
  }
  process_entity_deletions();
  this->clock().tick();
}

void Registry::update_bindings()
{
  for (auto& binding : _bindings) {

    binding.updater();
  }
}

void Registry::clear_bindings()
{
  _bindings.clear();
}

void Registry::emit(std::string const& name, JsonObject const& args)
{
  std::type_index type_id = _events_index_getter.at_second(name);
  if (!_event_handlers.contains(type_id)) {
    return;
  }

  auto builder =
      std::any_cast<std::function<std::any(Registry&, JsonObject const&)>>(
          _event_builders.at(type_id));
  std::any event = builder(*this, args);

  auto invoker =
      std::any_cast<std::function<void(const std::any&, const std::any&)>>(
          _event_invokers.at(type_id));
  invoker(_event_handlers.at(type_id), event);
}

void Registry::emit(std::string const& name, ByteArray const& data)
{
  if (!this->_byte_event_emitter.contains(name)) {
    return;
  }
  this->_byte_event_emitter.at(name)(data);
}

void Registry::add_scene(std::string const& scene_name, SceneState state)
{
  _scenes.insert_or_assign(scene_name, state);
}

void Registry::init_scene_management()
{
  this->register_component<Scene>("scene");
}

void Registry::setup_scene_systems()
{
  for (const auto& [name, state] : _scenes) {
    if (state == SceneState::MAIN) {
      _current_scene.push_back(name);
      break;
    }
    if (state == SceneState::ACTIVE) {
      _current_scene.push_back(name);
    }
  }
}

void Registry::set_current_scene(std::string const& scene_name)
{
  _current_scene.push_back(scene_name);
}

void Registry::remove_current_scene(std::string const& scene_name)
{
  _current_scene.erase(
      std::remove(_current_scene.begin(), _current_scene.end(), scene_name),
      _current_scene.end());
}

void Registry::remove_all_scenes()
{
  this->_current_scene.clear();
}

std::vector<std::string> const& Registry::get_current_scene() const
{
  return _current_scene;
}

Clock& Registry::clock()
{
  return _clock;
}

const Clock& Registry::clock() const
{
  return _clock;
}

void Registry::add_template(std::string const& name, JsonObject const& config)
{
  _entities_templates.insert_or_assign(name, config);
}

JsonObject Registry::get_template(std::string const& name)
{
  if (!_entities_templates.contains(name)) {
    std::cerr << "Template: " << name << " not found !\n";
  }
  return _entities_templates.find(name)->second;
}

bool Registry::is_in_current_cene(Entity e)
{
  return std::find(this->_current_scene.begin(),
                   this->_current_scene.end(),
                   this->get_components<Scene>()[e].value().scene_name)
      != this->_current_scene.end();
}

ByteArray Registry::convert_event_entity(
    std::string const& id,
    ByteArray const& event,
    std::unordered_map<Entity, Entity> const& map)
{
  if (!this->_event_entity_converters.contains(id)) {
    return event;
  }
  return this->_event_entity_converters.at(id)(event, map);
}

ByteArray Registry::convert_comp_entity(
    std::string const& id,
    ByteArray const& comp,
    std::unordered_map<Entity, Entity> const& map)
{
  return this->_comp_entity_converters.at(id)(comp, map);
}

std::vector<ComponentState> Registry::get_state()
{
  std::vector<ComponentState> r(this->_state_getters.size());
  for (auto const& it : this->_components) {
    r.emplace_back(this->_state_getters.at(it.first)());
  }
  return r;
}

ByteArray Registry::get_event_with_id(std::string const& id,
                                      JsonObject const& params)
{
  return this->_event_json_builder.at(this->_events_index_getter.at_second(id))(
      params);
}
