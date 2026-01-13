#pragma once

#include <optional>

#include "Json/JsonParser.hpp"
#include "ecs/EventManager.hpp"
#include "ecs/Registry.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/components/AnimatedSprite.hpp"
#include "plugin/events/CameraEvents.hpp"
#include "plugin/events/IoEvents.hpp"

class UI : public APlugin
{
public:
  UI(Registry& r,
     EventManager& em,
     EntityLoader& l,
     std::optional<JsonObject> const& config);
  ~UI() override = default;

private:
  void init_input(Registry::Entity entity, const JsonVariant& config);
  void init_drawable(Registry::Entity const& entity, JsonObject const& obj);
  void init_sprite(Registry::Entity const& entity, JsonObject const& obj);
  void init_text(Registry::Entity const& entity, JsonObject const& obj);
  void init_cam(Registry::Entity const& entity, JsonObject const& obj);
  void init_slider(const Registry::Entity& e, const JsonObject& obj);
  void init_background(Registry::Entity const& entity, JsonObject const& obj);
  void init_animated_sprite(Registry::Entity const& entity,
                            const JsonObject& obj);
  std::optional<AnimationData> parse_animation_data(JsonObject const& obj,
                                                    Registry::Entity const& e);

  void cam_target_event(const CamAggroEvent& e);
  void cam_move_event(const CamMoveEvent& e);
  void cam_zoom_event(const CamZoomEvent& e);
  void cam_rotate_event(const CamRotateEvent& e);
  void cam_speed_event(const CamSpeedEvent& e);

  void update_anim_system(Registry& r);

  void handle_key_pressed(const KeyPressedEvent& event);
  void handle_input_focus(const InputFocusEvent& event);
};

void on_click_slider(Registry& r, const MousePressedEvent& event);
void on_release_slider(Registry& r, const MouseReleasedEvent& event);
