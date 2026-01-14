#include "plugin/components/Input.hpp"

#include "UI.hpp"
#include "ecs/zipper/Zipper.hpp"
#include "ecs/zipper/ZipperIndex.hpp"
#include "plugin/components/AnimatedSprite.hpp"
#include "plugin/components/Drawable.hpp"
#include "plugin/events/AnimationEvents.hpp"

void UI::input_system(Registry& r)
{
  for (const auto&& [e, draw, input, anim] :
       ZipperIndex<Drawable, Input, AnimatedSprite>(r))
  {
    if (!draw.enabled) {
      continue;
    }
    if (input.enabled && anim.animations.contains("input_focus")) {
      this->_event_manager.get().emit<PlayAnimationEvent>(
          "input_focus",
          e,
          anim.animations.at("input_focus").framerate,
          false,
          false);
    } else if (!input.enabled && anim.animations.contains("idle")) {
      this->_event_manager.get().emit<PlayAnimationEvent>(
          "idle", e, anim.animations.at("idle").framerate, true, false);
    }
  }
}
