#include <algorithm>
#include <cstddef>

#include "plugin/components/BasicMap.hpp"

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/PrimitiveType.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <SFML/System/Vector2.hpp>

#include "SFMLRenderer.hpp"
#include "ecs/Registry.hpp"
#include "ecs/zipper/Zipper.hpp"
#include "libs/Color.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/components/Position.hpp"
#include "plugin/components/RaycastingCamera.hpp"
#include "plugin/events/RaycastingCameraEvents.hpp"

#define TO_RAD(degrees) ((degrees) * M_PI / 180.0)

namespace {

struct RayHit
{
  int mapx;
  int mapy;
  int side;
  double perp_wall_dist;
  Vector2D ray_dir;
};

RayHit cast_single_ray(Vector2D const& cam_pos,
                       Vector2D const& ray_dir,
                       std::vector<std::vector<int>> const& map_data)
{
  int mapx = static_cast<int>(cam_pos.x);
  int mapy = static_cast<int>(cam_pos.y);

  Vector2D delta_dist = {ray_dir.x == 0 ? 1e30 : std::abs(1 / ray_dir.x),
                         ray_dir.y == 0 ? 1e30 : std::abs(1 / ray_dir.y)};

  int step_x = (ray_dir.x < 0) ? -1 : 1;
  int step_y = (ray_dir.y < 0) ? -1 : 1;

  Vector2D side_dist;
  if (ray_dir.x < 0) {
    side_dist.x = (cam_pos.x - mapx) * delta_dist.x;
  } else {
    side_dist.x = (mapx + 1.0 - cam_pos.x) * delta_dist.x;
  }
  if (ray_dir.y < 0) {
    side_dist.y = (cam_pos.y - mapy) * delta_dist.y;
  } else {
    side_dist.y = (mapy + 1.0 - cam_pos.y) * delta_dist.y;
  }

  bool hit = false;
  int side = 0;

  while (!hit) {
    if (side_dist.x < side_dist.y) {
      side_dist.x += delta_dist.x;
      mapx += step_x;
      side = 0;
    } else {
      side_dist.y += delta_dist.y;
      mapy += step_y;
      side = 1;
    }
    if (mapy < 0 || mapy >= static_cast<int>(map_data.size()) ||
        mapx < 0 || mapx >= static_cast<int>(map_data[0].size())) {
      hit = true;
    } else if (map_data[mapy][mapx] > 0) {
      hit = true;
    }
  }

  double perp_wall_dist = (side == 0) ? side_dist.x - delta_dist.x
                                      : side_dist.y - delta_dist.y;
  perp_wall_dist = std::max(perp_wall_dist, 0.001);

  return {mapx, mapy, side, perp_wall_dist, ray_dir};
}

double compute_wall_x(Vector2D const& cam_pos, RayHit const& hit)
{
  double wall_x = (hit.side == 0)
      ? cam_pos.y + hit.perp_wall_dist * hit.ray_dir.y
      : cam_pos.x + hit.perp_wall_dist * hit.ray_dir.x;
  return wall_x - std::floor(wall_x);
}

std::string get_wall_direction(RayHit const& hit)
{
  if (hit.side == 0) {
    return (hit.ray_dir.x > 0) ? "west" : "east";
  }
  return (hit.ray_dir.y > 0) ? "north" : "south";
}

sf::Color get_fallback_color(int wall_value, int side)
{
  Color color;
  switch (wall_value) {
    case 1: color = RED; break;
    case 2: color = GREEN; break;
    case 3: color = BLUE; break;
    case 4: color = WHITE; break;
    default: color = YELLOW; break;
  }
  if (side == 1) {
    color.r /= 2;
    color.g /= 2;
    color.b /= 2;
  }
  return sf::Color(color.r, color.g, color.b, color.a);
}

void add_textured_quad(std::vector<sf::Vertex>& vertices,
                       float screen_x, float next_screen_x,
                       float y_start, float y_end,
                       TileData const& tile,
                       double wall_x, int side,
                       int line_height, int window_height)
{
  float tex_x = static_cast<float>(wall_x * tile.size.x);
  sf::Color color = (side == 1) ? sf::Color(180, 180, 180) : sf::Color::White;

  float tex_left = static_cast<float>(tile.pos.x) + tex_x;
  float tex_right = tex_left + 1.0f;

  int wall_top = (window_height - line_height) / 2;
  int wall_bottom = (window_height + line_height) / 2;

  float tex_y_start = static_cast<float>(tile.pos.y) +
      (y_start - wall_top) * static_cast<float>(tile.size.y) / line_height;
  float tex_y_end = static_cast<float>(tile.pos.y) +
      (y_end - wall_top) * static_cast<float>(tile.size.y) / line_height;

  vertices.emplace_back(sf::Vector2f(screen_x, y_start), color, sf::Vector2f(tex_left, tex_y_start));
  vertices.emplace_back(sf::Vector2f(next_screen_x, y_start), color, sf::Vector2f(tex_right, tex_y_start));
  vertices.emplace_back(sf::Vector2f(screen_x, y_end), color, sf::Vector2f(tex_left, tex_y_end));
  vertices.emplace_back(sf::Vector2f(next_screen_x, y_start), color, sf::Vector2f(tex_right, tex_y_start));
  vertices.emplace_back(sf::Vector2f(next_screen_x, y_end), color, sf::Vector2f(tex_right, tex_y_end));
  vertices.emplace_back(sf::Vector2f(screen_x, y_end), color, sf::Vector2f(tex_left, tex_y_end));
}

void add_colored_quad(std::vector<sf::Vertex>& vertices,
                      float screen_x, float next_screen_x,
                      float y_start, float y_end,
                      sf::Color const& color)
{
  vertices.emplace_back(sf::Vector2f(screen_x, y_start), color);
  vertices.emplace_back(sf::Vector2f(next_screen_x, y_start), color);
  vertices.emplace_back(sf::Vector2f(screen_x, y_end), color);
  vertices.emplace_back(sf::Vector2f(next_screen_x, y_start), color);
  vertices.emplace_back(sf::Vector2f(next_screen_x, y_end), color);
  vertices.emplace_back(sf::Vector2f(screen_x, y_end), color);
}

}  // namespace

void SFMLRenderer::basic_map_system(Registry& /*r*/) const {}

void SFMLRenderer::cast_rays(Registry& /*r*/,
                             RaycastingData const& data,
                             std::vector<std::vector<int>> const& map_data,
                             const sf::Vector2u& window_size)
{
  Vector2D dir = {std::cos(data.cam_angle), std::sin(data.cam_angle)};
  double plane_len = std::tan(TO_RAD(data.fov / 2));
  Vector2D plane = {-dir.y * plane_len, dir.x * plane_len};

  std::unordered_map<std::string, std::vector<sf::Vertex>> textured_vertices;
  std::vector<sf::Vertex> colored_vertices;

  for (int ray_idx = 0; ray_idx < data.nb_rays; ++ray_idx) {
    double camera_x = (2 * ray_idx / static_cast<double>(data.nb_rays)) - 1;
    Vector2D ray_dir = {dir.x + plane.x * camera_x, dir.y + plane.y * camera_x};

    RayHit hit = cast_single_ray(data.cam_pos, ray_dir, map_data);

    int line_height = static_cast<int>(window_size.y / hit.perp_wall_dist);
    int draw_start = std::max((-line_height / 2) + static_cast<int>(window_size.y / 2), 0);
    int draw_end = std::min((line_height / 2) + static_cast<int>(window_size.y / 2),
                            static_cast<int>(window_size.y) - 1);

    int wall_value = 0;
    if (hit.mapy >= 0 && hit.mapy < static_cast<int>(map_data.size()) &&
        hit.mapx >= 0 && hit.mapx < static_cast<int>(map_data[0].size())) {
      wall_value = map_data[hit.mapy][hit.mapx];
    }

    float screen_x = static_cast<float>(ray_idx) * window_size.x / data.nb_rays;
    float next_screen_x = static_cast<float>(ray_idx + 1) * window_size.x / data.nb_rays;
    float y_start = static_cast<float>(draw_start);
    float y_end = static_cast<float>(draw_end);

    double wall_x = compute_wall_x(data.cam_pos, hit);
    std::string wall_dir = get_wall_direction(hit);

    bool has_texture = data.tiles_data.contains(wall_value) &&
                       data.tiles_data.at(wall_value).contains(wall_dir);

    if (has_texture) {
      const TileData& tile = data.tiles_data.at(wall_value).at(wall_dir);
      add_textured_quad(textured_vertices[tile.texture_path],
                        screen_x, next_screen_x, y_start, y_end,
                        tile, wall_x, hit.side,
                        line_height, static_cast<int>(window_size.y));
    } else {
      sf::Color color = get_fallback_color(wall_value, hit.side);
      add_colored_quad(colored_vertices, screen_x, next_screen_x, y_start, y_end, color);
    }
  }

  draw_textured_walls(textured_vertices);
  draw_colored_walls(colored_vertices);
}

void SFMLRenderer::draw_textured_walls(
    std::unordered_map<std::string, std::vector<sf::Vertex>>& textured_vertices)
{
  for (auto& [texture_path, vertices] : textured_vertices) {
    sf::VertexArray va(sf::PrimitiveType::Triangles, vertices.size());
    for (size_t i = 0; i < vertices.size(); ++i) {
      va[i] = vertices[i];
    }
    sf::RenderStates states;
    states.texture = &this->load_texture(texture_path);
    this->_window.draw(va, states);
  }
}

void SFMLRenderer::draw_colored_walls(std::vector<sf::Vertex>& colored_vertices)
{
  if (colored_vertices.empty()) {
    return;
  }
  sf::VertexArray va(sf::PrimitiveType::Triangles, colored_vertices.size());
  for (size_t i = 0; i < colored_vertices.size(); ++i) {
    va[i] = colored_vertices[i];
  }
  this->_window.draw(va);
}

void SFMLRenderer::render_basic_map(Registry& r,
                                    std::vector<DrawableItem>& /*all_drawables*/,
                                    float /*min_dimension*/,
                                    const sf::Vector2u& window_size)
{
  Vector2D cam_pos;
  double cam_angle = 0;
  double fov = 0;
  int nb_rays = 0;

  for (auto&& [pos, cam] : Zipper<Position, RaycastingCamera>(r)) {
    cam_angle = cam.angle;
    cam_pos = pos.pos;
    fov = cam.fov;
    nb_rays = cam.nb_rays;
  }

  if (fov == 0) {
    return;
  }

  for (auto&& [draw, basic_map] : Zipper<Drawable, BasicMap>(r)) {
    if (!draw.enabled) {
      continue;
    }
    RaycastingData raycasting_data {
        .tiles_data = basic_map.tiles_data,
        .cam_pos = cam_pos,
        .map_size = {static_cast<double>(basic_map.data[0].size()),
                     static_cast<double>(basic_map.data.size())},
        .cam_angle = cam_angle,
        .fov = fov,
        .angle_step = fov / nb_rays,
        .nb_rays = nb_rays};
    this->cast_rays(r, raycasting_data, basic_map.data, window_size);
  }
}
