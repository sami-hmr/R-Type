

#include <optional>
#include <vector>

#include "ecs/Registry.hpp"

#include "NetworkShared.hpp"
#include "ecs/EventManager.hpp"
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
    for (auto it = _bindings.begin(); it != _bindings.end();) {
      if (it->target_entity == e) {
        it->deleter();
        it = _bindings.erase(it);
      } else {
        ++it;
      }
    }

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

void Registry::run_systems(EventManager& em)
{
  update_bindings(em);

  std::vector<System<>> pending = this->_frequent_systems;
  for (auto const& f : pending) {
    f();
  }
  process_entity_deletions();
  this->clock().tick();
}

void Registry::update_bindings(EventManager& em)
{
  for (auto& binding : _bindings) {
    binding.updater();
    ByteArray component_data = binding.serializer();

    if (!component_data.empty()) {
      em.emit<ComponentBuilder>(
          binding.target_entity,
          this->_index_getter.at_first(binding.target_component),
          component_data);
    }
  }
}

void Registry::clear_bindings()
{
  for (auto& binding : _bindings) {
    binding.deleter();
  }
  _bindings.clear();
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

void Registry::delete_systems() {
    this->_frequent_systems.clear();
}
