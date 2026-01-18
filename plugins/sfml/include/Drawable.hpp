#pragma once

#include <functional>
#include <optional>
#include <utility>

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <SFML/System/Vector2.hpp>

#include "libs/Color.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/components/AnimatedSprite.hpp"

struct AnimatedSpriteDrawable
{
  std::reference_wrapper<sf::Sprite> sprite;
  std::string texture_name;
  sf::Vector2f pos;
  sf::Vector2f scale;
  AnimationData animdata;
  float rotation = 0;
  int z_index;

  AnimatedSpriteDrawable(std::reference_wrapper<sf::Sprite> spr,
                         std::string texture_name,
                         sf::Vector2f p,
                         sf::Vector2f s,
                         AnimationData ad,
                         float rot,
                         int z)
      : sprite(spr)
      , texture_name(std::move(texture_name))
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
  std::string texture_name;
  sf::Vector2f pos;
  sf::Vector2f scale;
  float rotation = 0;
  int z_index;

  SpriteDrawable(std::reference_wrapper<sf::Sprite> spr,
                 std::string texture_name,
                 sf::Vector2f p,
                 sf::Vector2f s,
                 float rot,
                 int z)
      : sprite(spr)
      , texture_name(std::move(texture_name))
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
  std::string font_name;
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
               std::string font_name,
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
      , font_name(std::move(font_name))
      , text_str(std::move(text_str))
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

static const std::string abc =
    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

struct BarDrawable
{
  std::reference_wrapper<sf::RectangleShape> rectangle;
  sf::Vector2f pos;
  sf::Vector2f size;
  Color fill_color;
  float fill_percentage;
  std::string texture_name;
  int z_index;
  bool outline;

  BarDrawable(std::reference_wrapper<sf::RectangleShape> rectangle,
              sf::Vector2f pos,
              sf::Vector2f size,
              Color fill_color,
              float fill_percentage,
              std::string texture_name,
              int z_index,
              bool outline)
      : rectangle(rectangle)
      , pos(pos)
      , size(size)
      , fill_color(fill_color)
      , fill_percentage(fill_percentage)
      , texture_name(std::move(texture_name))
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

  SliderDrawable(std::reference_wrapper<sf::RectangleShape> rectangle,
                 std::reference_wrapper<sf::CircleShape> circle,
                 sf::Vector2f pos,
                 sf::Vector2f circle_pos,
                 sf::Vector2f size,
                 Color bar_color,
                 Color circle_color,
                 float radius,
                 int z_index)
      : rectangle(rectangle)
      , circle(circle)
      , pos(pos)
      , circle_pos(circle_pos)
      , size(size)
      , bar_color(bar_color)
      , circle_color(circle_color)
      , radius(radius)
      , z_index(z_index)
  {
  }
};

struct TriangleVerticesDrawable
{
  sf::Vertex p1;
  sf::Vertex p2;
  sf::Vertex p3;
  int z_index = -1;

  TriangleVerticesDrawable(sf::Vertex vertex1,
                           sf::Vertex vertex2,
                           sf::Vertex vertex3,
                           int z)
      : p1(vertex1)
      , p2(vertex2)
      , p3(vertex3)
      , z_index(z)
  {
  }
};

using DrawableVariant = std::variant<AnimatedSpriteDrawable,
                                     SpriteDrawable,
                                     BarDrawable,
                                     SliderDrawable,
                                     TriangleVerticesDrawable,
                                     TextDrawable>;

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
    return z_index == other.z_index ? drawable.index() < other.drawable.index()
                                    : z_index < other.z_index;
  }

  void draw(sf::RenderWindow& window,
            std::unordered_map<std::string, sf::Texture>& textures,
            std::unordered_map<std::string, sf::Font>& fonts);
};
