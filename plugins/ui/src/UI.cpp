#include "UI.hpp"

#include "ecs/EventManager.hpp"
#include "ecs/InitComponent.hpp"
#include "ecs/Registry.hpp"
#include "ecs/zipper/Zipper.hpp"
#include "libs/Color.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/components/AnimatedSprite.hpp"
#include "plugin/components/Background.hpp"
#include "plugin/components/Camera.hpp"
#include "plugin/components/Drawable.hpp"
#include "plugin/components/Input.hpp"
#include "plugin/components/Slider.hpp"
#include "plugin/components/Sprite.hpp"
#include "plugin/components/Text.hpp"
#include "plugin/events/IoEvents.hpp"

UI::UI(Registry& r,
       EventManager& em,
       EntityLoader& l,
       std::optional<JsonObject> const& config)
    : APlugin("ui",
              r,
              em,
              l,
              {},
              {COMP_INIT(input, Input, init_input),
               COMP_INIT(Drawable, Drawable, init_drawable),
               COMP_INIT(Sprite, Sprite, init_sprite),
               COMP_INIT(Text, Text, init_text),
               COMP_INIT(Camera, Camera, init_cam),
               COMP_INIT(Background, Background, init_background),
               COMP_INIT(AnimatedSprite, AnimatedSprite, init_animated_sprite),
               COMP_INIT(Slider, Slider, init_slider)},
              config)
{
  SUBSCRIBE_EVENT(KeyPressedEvent, { this->handle_key_pressed(event); })

  REGISTER_COMPONENT(Drawable)
  REGISTER_COMPONENT(Input)
  REGISTER_COMPONENT(Sprite)
  REGISTER_COMPONENT(Text)
  REGISTER_COMPONENT(Camera)
  REGISTER_COMPONENT(Background)
  REGISTER_COMPONENT(AnimatedSprite)
  REGISTER_COMPONENT(Slider)

  this->_registry.get().add_system(
      [this](Registry& r) { this->update_anim_system(r); }, 1000);
    this->_registry.get().add_system([this] (Registry& r) { this->input_system(r); }, 1000);

  SUBSCRIBE_EVENT(CamAggroEvent, { this->cam_target_event(event); })
  SUBSCRIBE_EVENT(CamZoomEvent, { this->cam_zoom_event(event); })
  SUBSCRIBE_EVENT(CamRotateEvent, { this->cam_rotate_event(event); })
  SUBSCRIBE_EVENT(CamSpeedEvent, { this->cam_speed_event(event); })
  SUBSCRIBE_EVENT(CamMoveEvent, { this->cam_move_event(event); })
  SUBSCRIBE_EVENT(PlayAnimationEvent, {
    AnimatedSprite::on_play_animation(
        this->_registry.get(), this->_event_manager.get(), event);
  })
  SUBSCRIBE_EVENT(AnimationEndEvent, {
    AnimatedSprite::on_animation_end(
        this->_registry.get(), this->_event_manager.get(), event);
  })
  SUBSCRIBE_EVENT(DeathEvent, {
    AnimatedSprite::on_death(
        this->_registry.get(), this->_event_manager.get(), event);
  })

  SUBSCRIBE_EVENT(MousePressedEvent, { disable_all_inputs(); })

  SUBSCRIBE_EVENT(MousePressedEvent,
                  { on_click_slider(this->_registry.get(), event); });
  SUBSCRIBE_EVENT(MouseReleasedEvent,
                  { on_release_slider(this->_registry.get(), event); });

}


void UI::disable_all_inputs() {
  for (auto &&[input] : Zipper<Input>(this->_registry.get())) {
    input.enabled = false;
  }
}

extern "C"
{
PLUGIN_EXPORT void* entry_point(Registry& r,
                  EventManager& em,
                  EntityLoader& l,
                  std::optional<JsonObject> const& config)
{
  return new UI(r, em, l, config);
}
}
