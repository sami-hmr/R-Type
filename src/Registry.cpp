

#include <functional>
#include <optional>
#include <utility>
#include <vector>

#include "ecs/Registry.hpp"

#include "Json/JsonParser.hpp"
#include "NetworkShared.hpp"
#include "ecs/EventManager.hpp"
#include "ecs/JsonTemplateUtils.hpp"
#include "ecs/Scenes.hpp"
#include "ecs/Systems.hpp"
#include "plugin/Byte.hpp"

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
    // Only emit network event if value actually changed
    if (binding.update_and_check_dirty()) {
      if (!binding.last_serialized_value.empty()) {
        em.emit<ComponentBuilder>(
            binding.target_entity,
            this->_index_getter.at_first(binding.target_component),
            binding.last_serialized_value);
      }
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
  if (state == SceneState::ACTIVE || state == SceneState::MAIN) {
    activate_scene(scene_name);
  }
  if (state == SceneState::MAIN) {
    set_main_scene(scene_name);
  }
}

void Registry::init_scene_management()
{
  this->register_component<Scene>("scene");
}

void Registry::setup_scene_systems()
{
  // Activate all scenes that were registered as ACTIVE or MAIN
  for (const auto& [name, state] : _scenes) {
    if (state == SceneState::ACTIVE || state == SceneState::MAIN) {
      activate_scene(name);
    }
  }
}

void Registry::activate_scene(std::string const& scene_name)
{
  if (!_scenes.contains(scene_name)) {
    _scenes.insert({scene_name, SceneState::DISABLED});
  }

  // If the scene is not already MAIN, set it to ACTIVE
  if (_scenes[scene_name] != SceneState::MAIN) {
    _scenes[scene_name] = SceneState::ACTIVE;
  }

  if (_active_scenes_set.contains(scene_name)) {
    return;
  }

  _current_scene.push_back(scene_name);
  _active_scenes_set.insert(scene_name);
}

void Registry::deactivate_scene(std::string const& scene_name)
{
  if (_scenes.contains(scene_name)) {
    _scenes[scene_name] = SceneState::DISABLED;
  }

  _active_scenes_set.erase(scene_name);
  _current_scene.erase(
      std::remove(_current_scene.begin(), _current_scene.end(), scene_name),
      _current_scene.end());
  this->_main_scene.remove(scene_name);
}

void Registry::deactivate_all_scenes()
{
  for (auto& [name, state] : _scenes) {
    state = SceneState::DISABLED;
  }
  _current_scene.clear();
  _active_scenes_set.clear();
}

void Registry::push_scene(std::string const& scene_name)
{
  activate_scene(scene_name);
}

void Registry::pop_scene(std::string const& scene_name)
{
  deactivate_scene(scene_name);
}

bool Registry::is_scene_active(std::string const& scene_name) const
{
  return _active_scenes_set.contains(scene_name);
}

SceneState Registry::get_scene_state(std::string const& scene_name) const
{
  auto it = _scenes.find(scene_name);
  return (it != _scenes.end()) ? it->second : SceneState::DISABLED;
}

std::unordered_set<std::string> const& Registry::get_active_scenes_set() const
{
  return _active_scenes_set;
}

std::unordered_map<std::string, SceneState> const& Registry::get_scene_states()
    const
{
  return _scenes;
}

std::vector<std::string> const& Registry::get_active_scenes() const
{
  return _current_scene;
}

// Deprecated methods for backward compatibility
void Registry::set_current_scene(std::string const& scene_name)
{
  activate_scene(scene_name);
}

void Registry::set_main_scene(std::string const& scene_name)
{
  this->_main_scene.push_front(scene_name);
}

void Registry::remove_current_scene(std::string const& scene_name)
{
  deactivate_scene(scene_name);
}

void Registry::remove_all_scenes()
{
  deactivate_all_scenes();
}

std::vector<std::string> const& Registry::get_current_scene() const
{
  return _current_scene;
}

bool Registry::is_in_main_scene(Entity e)
{
  if (!this->has_component<Scene>(e)) {
    return true;
  }
  return this->_main_scene.empty()
      || this->get_components<Scene>()[e]->scene_name
      == this->_main_scene.front();
}

Clock& Registry::clock()
{
  return _clock;
}

const Clock& Registry::clock() const
{
  return _clock;
}

void Registry::add_template(std::string const& name,
                            JsonObject const& config,
                            JsonObject const& default_parameters)
{
  _entities_templates.insert_or_assign(
      name, TemplateDefinition(config, default_parameters));
}

JsonObject Registry::get_template(std::string const& name,
                                  JsonObject const& params)
{
  if (!_entities_templates.contains(name)) {
    std::cerr << "Template: " << name << " not found !\n";
  }
  auto const& definition = _entities_templates.find(name)->second;
  JsonObject new_object = definition.obj;
  if (params.empty() && definition.default_parameters.empty()) {
    return new_object;
  }
  replace_json_object(new_object, params, definition.default_parameters);
  return new_object;
}

bool Registry::is_in_current_cene(Entity e)
{
  if (!this->has_component<Scene>(e)) {
    return false;
  }
  return _active_scenes_set.contains(
      this->get_components<Scene>()[e].value().scene_name);
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

ByteArray Registry::get_byte_entity(Entity entity)
{
  std::vector<std::pair<std::string, ByteArray>> entity_vector;
  for (auto const& it : this->_component_getter) {
    auto comp = it.second(entity);

    if (!comp) {
      continue;
    }
    entity_vector.emplace_back(this->_index_getter.at_first(it.first), *comp);
  }

  return vector_to_byte(
      entity_vector,
      std::function<ByteArray(std::pair<std::string, ByteArray> const&)>(
          [](std::pair<std::string, ByteArray> const& v)
          {
            return pair_to_byte(
                v,
                std::function(string_to_byte),
                std::function<ByteArray(ByteArray const&)>(
                    [](ByteArray const& v)
                    { return vector_to_byte(v, TTB_FUNCTION<Byte>()); }));
          }));
}
