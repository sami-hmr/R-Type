
#include "Json/JsonParser.hpp"
#include "NetworkShared.hpp"
#include "ecs/Registry.hpp"
#include "plugin/events/LoggerEvent.hpp"


inline void emit_event(Registry& r, std::string const &id, JsonObject const &params)
{
  try {
    r.emit<EventBuilder>(EventBuilder(id, r.get_event_with_id(id, params)));
  } catch (std::out_of_range const&) {
    r.emit<LogEvent>("init", LogLevel::ERROR, "unknow event");
  }
  r.emit(id, params);
}

template<event Event>
void emit_event(Registry& r, std::string const &id, Event event)
{
  try {
      r.emit<EventBuilder>(EventBuilder(id, event.to_bytes()));
  } catch (std::out_of_range const&) {
    r.emit<LogEvent>("init", LogLevel::ERROR, "unknow event");
  }
  r.emit(id, event.to_bytes());
}
