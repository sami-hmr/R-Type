
#include "Drawable.hpp"

#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/System/Angle.hpp>
#include <SFML/System/Vector2.hpp>

static const double deux = 2.0;

static void update(AnimatedSpriteDrawable& drawable)
{
  drawable.sprite.get().setTexture(drawable.texture.get(), true);
  drawable.sprite.get().setOrigin(
      sf::Vector2f(static_cast<float>(drawable.animdata.frame_size.x) / deux,
                   static_cast<float>(drawable.animdata.frame_size.y) / deux));
  drawable.sprite.get().setTextureRect(
      sf::IntRect({static_cast<int>(drawable.animdata.frame_pos.x),
                   static_cast<int>(drawable.animdata.frame_pos.y)},
                  {static_cast<int>(drawable.animdata.frame_size.x),
                   static_cast<int>(drawable.animdata.frame_size.y)}));
  drawable.sprite.get().setScale(drawable.scale);
  drawable.sprite.get().setRotation(sf::degrees(drawable.rotation));
  drawable.sprite.get().setPosition(drawable.pos);
}

static void update(SpriteDrawable& drawable)
{
  sf::Vector2u texture_size = drawable.texture.get().getSize();

  drawable.sprite.get().setTexture(drawable.texture.get(), true);
  drawable.sprite.get().setOrigin(
      sf::Vector2f(static_cast<float>(texture_size.x) / deux,
                   static_cast<float>(texture_size.y) / deux));
  drawable.sprite.get().setScale(drawable.scale);
  drawable.sprite.get().setRotation(sf::degrees(drawable.rotation));
  drawable.sprite.get().setPosition(drawable.pos);
}

static void update(TextDrawable& drawable)
{
  drawable.text.get().setString(drawable.text_str);
  drawable.text.get().setFont(drawable.font.get());
  drawable.text.get().setCharacterSize(drawable.size);
  sf::FloatRect text_rect = drawable.text.get().getLocalBounds();
  drawable.text.get().setOrigin(
      {text_rect.position.x + (text_rect.size.x / 2.0f),
       text_rect.position.y + (text_rect.size.y / 2.0f)});
  drawable.text.get().setFillColor(sf::Color(drawable.fill_color.r,
                                             drawable.fill_color.g,
                                             drawable.fill_color.b,
                                             drawable.fill_color.a));
  if (drawable.outline) {
    drawable.text.get().setOutlineColor(sf::Color(drawable.outline_color.r,
                                                  drawable.outline_color.g,
                                                  drawable.outline_color.b,
                                                  drawable.outline_color.a));
    drawable.text.get().setOutlineThickness(drawable.outline_thickness);
  } else {
    drawable.text.get().setOutlineThickness(0.0f);
  }
  drawable.text.get().setRotation(sf::degrees(drawable.rotation));
  drawable.text.get().setPosition(drawable.pos);
}

static void update(BarDrawable& drawable, sf::RenderWindow& window)
{
  drawable.rectangle.get().setFillColor(sf::Color::Transparent);
  drawable.rectangle.get().setOutlineColor(sf::Color::Transparent);

  drawable.rectangle.get().setPosition(drawable.pos);
  drawable.rectangle.get().setSize(drawable.size);
  drawable.rectangle.get().setOrigin(
      {drawable.size.x / 2, drawable.size.y / 2});
  if (drawable.outline) {
    drawable.rectangle.get().setOutlineColor({drawable.fill_color.r,
                                              drawable.fill_color.g,
                                              drawable.fill_color.b,
                                              drawable.fill_color.a});
    drawable.rectangle.get().setOutlineThickness(drawable.size.y * 0.1f);
    window.draw(drawable.rectangle.get());
  }

  if (drawable.texture.has_value()) {
    sf::Vector2u texture_size = drawable.texture.value().getSize();
    drawable.rectangle.get().setTexture(&drawable.texture.value());
    drawable.rectangle.get().setTextureRect(sf::IntRect(
        {0, 0},
        {static_cast<int>(texture_size.x * drawable.fill_percentage),
         static_cast<int>(texture_size.y)}));
  } else {
    drawable.rectangle.get().setTexture(nullptr, true);
  }
  drawable.rectangle.get().setSize(
      {drawable.size.x * drawable.fill_percentage, drawable.size.y});
  drawable.rectangle.get().setOutlineColor(sf::Color::Transparent);
  drawable.rectangle.get().setOutlineThickness(0.0f);
  drawable.rectangle.get().setFillColor(sf::Color(drawable.fill_color.r,
                                                  drawable.fill_color.g,
                                                  drawable.fill_color.b,
                                                  drawable.fill_color.a));
  window.draw(drawable.rectangle.get());
}

static void update(SliderDrawable& drawable)
{
  drawable.rectangle.get().setFillColor(sf::Color(drawable.bar_color.r,
                                                  drawable.bar_color.g,
                                                  drawable.bar_color.b,
                                                  drawable.bar_color.a));
  drawable.rectangle.get().setSize(drawable.size);
  drawable.rectangle.get().setOrigin(
      {drawable.size.x / 2, drawable.size.y / 2});
  drawable.rectangle.get().setPosition(drawable.pos);

  drawable.circle.get().setFillColor(sf::Color(drawable.circle_color.r,
                                               drawable.circle_color.g,
                                               drawable.circle_color.b,
                                               drawable.circle_color.a));
  drawable.circle.get().setRadius(drawable.radius);
  drawable.circle.get().setOrigin({drawable.radius, drawable.radius});
  drawable.circle.get().setPosition(drawable.circle_pos);
}


void DrawableItem::draw(sf::RenderWindow& window)
{
  std::visit(
      [&window](auto&& d)
      {
        using T = std::decay_t<decltype(d)>;

        if constexpr (std::is_same_v<T, AnimatedSpriteDrawable>
                      || std::is_same_v<T, SpriteDrawable>)
        {
          update(d);
          window.draw(d.sprite.get());
        } else if constexpr (std::is_same_v<T, TextDrawable>) {
          update(d);
          window.draw(d.text.get());
        } else if constexpr (std::is_same_v<T, BarDrawable>) {
          update(d, window);
        } else if constexpr (std::is_same_v<T, SliderDrawable>) {
          update(d);
          window.draw(d.rectangle.get());
          window.draw(d.circle.get());
        }
      },
      drawable);
}
