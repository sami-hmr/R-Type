#include "UI.hpp"

#include "plugin/components/Input.hpp"
#include "plugin/events/Events.hpp"

UI::UI(Registry& r, EntityLoader& l, std::optional<JsonObject> const& config)
    : APlugin(r, l, {}, {COMP_INIT(input, Input, init_input)}, config)
{
<<<<<<< Updated upstream
  _registry.get().on<KeyPressedEvent>("KeyPressedEvent", [this](const KeyPressedEvent& event)
                                       { this->handle_key_pressed(event); });
=======
  _registry.get().on<KeyPressedEvent>("KeyPressedEvent",
                                      [this](const KeyPressedEvent& event)
                                      { this->handle_key_pressed(event); });
>>>>>>> Stashed changes
  _registry.get().register_component<Input>("input");
}

void UI::init_input(Registry::Entity entity, const JsonVariant& config)
{
  bool enabled = false;
  std::string buffer;

  try {
    JsonObject obj = std::get<JsonObject>(config);
    if (obj.contains("enabled")) {
      enabled = std::get<bool>(obj.at("enabled").value);
    }
    if (obj.contains("buffer")) {
      buffer = std::get<std::string>(obj.at("buffer").value);
    }

    _registry.get().emplace_component<Input>(entity, Input(enabled, buffer));
  } catch (std::bad_variant_access const&) {
  }
}

void UI::handle_key_pressed(const KeyPressedEvent& event)
{
  auto& inputs = _registry.get().get_components<Input>();

  for (auto& input : inputs) {
    if (!input.has_value()) {
      continue;
    }

    if (!input.value().enabled) {
      continue;
    }

    if (event.key_unicode.has_value()) {
      input.value().buffer += event.key_unicode.value();
    }

    if (event.key_pressed.contains(Key::DELETE)
        && event.key_pressed.at(Key::DELETE))
    {
      if (!input.value().buffer.empty()) {
        input.value().buffer.pop_back();
      }
    }
  }
}

extern "C"
{
void* entry_point(Registry& r,
                  EntityLoader& l,
                  std::optional<JsonObject> const& config)
{
  return new UI(r, l, config);
}
}
