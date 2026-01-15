#include <algorithm>
#include <cmath>

#include "plugin/components/Slider.hpp"

#include <SFML/System/Vector2.hpp>

#include "Drawable.hpp"
#include "SFMLRenderer.hpp"
#include "ecs/Registry.hpp"
#include "ecs/zipper/Zipper.hpp"
#include "plugin/components/Drawable.hpp"
#include "plugin/components/Position.hpp"

static const double deux = 2.0;

static const float deux_float = 2.0f;

void SFMLRenderer::render_sliders(Registry& r,
                                  std::vector<DrawableItem>& all_drawables,
                                  float min_dimension,
                                  const sf::Vector2u& window_size)
{
  for (const auto&& [draw, pos, slider] : Zipper<Drawable, Position, Slider>(r))
  {
    if (!draw.enabled) {
      continue;
    }
    float offset_x = (window_size.x - min_dimension) / deux_float;
    float offset_y = (window_size.y - min_dimension) / deux_float;

    sf::Vector2f new_pos(
        static_cast<float>((pos.pos.x + 1.0) * min_dimension / deux) + offset_x,
        static_cast<float>((pos.pos.y + 1.0) * min_dimension / deux)
            + offset_y);
    sf::Vector2f rect_size(static_cast<float>(slider.size.x * min_dimension),
                           static_cast<float>(slider.size.y * min_dimension));
    float radius = (rect_size.y / deux_float);
    float ratio = static_cast<float>((slider.current_value - slider.min_value)
                                     / (slider.max_value - slider.min_value));
    float left_edge = new_pos.x - (rect_size.x / deux_float);
    float circle_x =
        left_edge + radius + (ratio * (rect_size.x - deux_float * radius));
    sf::Vector2f circle_pos(circle_x, new_pos.y);
    SliderDrawable slider_drawable(std::ref(this->_rectangle),
                                   std::ref(this->_circle),
                                   new_pos,
                                   circle_pos,
                                   rect_size,
                                   slider.bar_color,
                                   slider.circle_color,
                                   radius,
                                   pos.z);
    DrawableVariant drawable_variant = std::move(slider_drawable);
    all_drawables.emplace_back(std::move(drawable_variant), pos.z);
  }
}

void SFMLRenderer::slider_system(Registry& r) const
{
  for (auto&& [pos, draw, slider] : Zipper<Position, Drawable, Slider>(r)) {
    if (!draw.enabled || !slider.selected) {
      continue;
    }
    double left_edge = pos.pos.x - slider.size.x;
    double right_edge = pos.pos.x + slider.size.x;
    double slider_width = right_edge - left_edge;

    double ratio = (_mouse_pos.x - left_edge) / slider_width;

    ratio = std::max(0.0, std::min(1.0, ratio));

    double new_value =
        slider.min_value + (ratio * (slider.max_value - slider.min_value));

    new_value =
        (std::round((new_value - slider.min_value) / slider.step) * slider.step)
        + slider.min_value;

    slider.current_value =
        std::max(slider.min_value, std::min(slider.max_value, new_value));
  }
}
