#pragma once

#include <functional>
#include <optional>
#include <utility>

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Vector2.hpp>

#include "libs/Color.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/components/AnimatedSprite.hpp"

struct AnimatedSpriteDrawable
{
  std::reference_wrapper<sf::Sprite> sprite;
  std::reference_wrapper<sf::Texture> texture;
  sf::Vector2f pos;
  sf::Vector2f scale;
  AnimationData animdata;
  float rotation = 0;
  int z_index;

  AnimatedSpriteDrawable(std::reference_wrapper<sf::Sprite> spr,
                         std::reference_wrapper<sf::Texture> tex,
                         sf::Vector2f p,
                         sf::Vector2f s,
                         AnimationData ad,
                         float rot,
                         int z)
      : sprite(spr)
      , texture(tex)
      , pos(p)
      , scale(s)
      , animdata(std::move(ad))
      , rotation(rot)
      , z_index(z)
  {
  }
};

struct SpriteDrawable
{
  std::reference_wrapper<sf::Sprite> sprite;
  std::reference_wrapper<sf::Texture> texture;
  sf::Vector2f pos;
  sf::Vector2f scale;
  float rotation = 0;
  int z_index;

  SpriteDrawable(std::reference_wrapper<sf::Sprite> spr,
                 std::reference_wrapper<sf::Texture> tex,
                 sf::Vector2f p,
                 sf::Vector2f s,
                 float rot,
                 int z)
      : sprite(spr)
      , texture(tex)
      , pos(p)
      , scale(s)
      , rotation(rot)
      , z_index(z)
  {
  }
};

struct TextDrawable
{
  std::reference_wrapper<sf::Text> text;
  std::reference_wrapper<sf::Font> font;
  std::string text_str;
  sf::Vector2f pos;
  Color fill_color;
  Color outline_color;
  float outline_thickness = 0.0f;
  float rotation = 0;
  int z_index;
  unsigned int size;
  bool outline = false;

  TextDrawable(std::reference_wrapper<sf::Text> t,
               std::reference_wrapper<sf::Font> f,
               std::string text_str,
               sf::Vector2f p,
               Color fill,
               Color outline_col,
               float outline_thick,
               float rot,
               int z,
               unsigned int s,
               bool outl)
      : text(t)
      , font(f)
      , text_str(text_str)
      , pos(p)
      , fill_color(fill)
      , outline_color(outline_col)
      , outline_thickness(outline_thick)
      , rotation(rot)
      , z_index(z)
      , size(s)
      , outline(outl)
  {
  }
};

struct BarDrawable
{
  std::reference_wrapper<sf::RectangleShape> rectangle;
  sf::Vector2f pos;
  sf::Vector2f size;
  Color fill_color;
  float fill_percentage;
  std::optional<sf::Texture> texture;
  int z_index;
  bool outline;

  BarDrawable(std::reference_wrapper<sf::RectangleShape> rectangle,
              sf::Vector2f pos,
              sf::Vector2f size,
              Color fill_color,
              float fill_percentage,
              std::optional<sf::Texture> texture,
              int z_index,
              bool outline)
      : rectangle(rectangle)
      , pos(pos)
      , size(size)
      , fill_color(fill_color)
      , fill_percentage(fill_percentage)
      , texture(std::move(texture))
      , z_index(z_index)
      , outline(outline)
  {
  }
};

struct SliderDrawable
{
  std::reference_wrapper<sf::RectangleShape> rectangle;
  std::reference_wrapper<sf::CircleShape> circle;
  sf::Vector2f pos;
  sf::Vector2f circle_pos;
  sf::Vector2f size;
  Color bar_color;
  Color circle_color;
  float radius;
  int z_index;
};

using DrawableVariant = std::
    variant<AnimatedSpriteDrawable, SpriteDrawable, TextDrawable, BarDrawable, SliderDrawable>;

struct DrawableItem
{
  DrawableVariant drawable;
  int z_index;

  DrawableItem(DrawableVariant d, int z)
      : drawable(std::move(d))
      , z_index(z)
  {
  }

  bool operator<(const DrawableItem& other) const
  {
    return z_index < other.z_index;
  }

  void draw(sf::RenderWindow& window);
};
