#include "ecs/EventManager.hpp"

#include "ecs/Registry.hpp"

void EventManager::emit(Registry& r,
                        std::string const& name,
                        JsonObject const& args,
                        std::optional<Ecs::Entity> entity)
{
  std::type_index type_id = _index_getter.at_second(name);
  if (!_handlers.contains(type_id)) {
    return;
  }

  auto builder = std::any_cast<std::function<std::any(
      Registry&, JsonObject const&, std::optional<Ecs::Entity>)>>(
      _builders.at(type_id));
  std::any event = builder(r, args, entity);

  auto invoker =
      std::any_cast<std::function<void(const std::any&, const std::any&)>>(
          _invokers.at(type_id));
  invoker(_handlers.at(type_id), event);
}

void EventManager::emit(std::string const& name, ByteArray const& data)
{
  if (!this->_byte_emitter.contains(name)) {
    return;
  }
  this->_byte_emitter.at(name)(data);
}

ByteArray EventManager::convert_event_entity(
    std::string const& id,
    ByteArray const& event,
    std::unordered_map<Ecs::Entity, Ecs::Entity> const& map)
{
  if (!this->_entity_converter.contains(id)) {
    return event;
  }
  return this->_entity_converter.at(id)(event, map);
}

ByteArray EventManager::get_event_with_id(Registry& r,
                                          std::string const& id,
                                          JsonObject const& params,
                                          std::optional<Ecs::Entity> entity)
{
  return this->_json_builder.at(this->_index_getter.at_second(id))(
      r, params, entity);
}

void EventManager::delete_all()
{
  this->_handlers.clear();
  this->_byte_emitter.clear();
  this->_builders.clear();
  this->_json_builder.clear();
  this->_entity_converter.clear();
  this->_invokers.clear();
}
