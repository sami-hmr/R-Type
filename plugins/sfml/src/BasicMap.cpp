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

void SFMLRenderer::basic_map_system(Registry& r) const
{
  return;
}

/**
 * @brief cast rays. Camera angle in radians, fov in degrees
 *
 * @param r
 * @param raycasting_data
 * @param map_data
 * @param window_size
 */
void SFMLRenderer::cast_rays(Registry& /*r*/,
                             RaycastingData const& raycasting_data,
                             std::vector<std::vector<int>> const& map_data,
                             const sf::Vector2u& window_size)
{
  Vector2D dir = {std::cos(raycasting_data.cam_angle),
                  std::sin(raycasting_data.cam_angle)};
  double plane_len = std::tan(TO_RAD(raycasting_data.fov / 2));
  Vector2D plane = {-dir.y * plane_len, dir.x * plane_len};

  for (int ray_idx = 0; ray_idx < raycasting_data.nb_rays; ++ray_idx) {
    double camera_x =
        (2 * ray_idx / static_cast<double>(raycasting_data.nb_rays)) - 1;

    Vector2D ray_dir = {dir.x + plane.x * camera_x,
                        dir.y + plane.y * camera_x};

    int mapx = static_cast<int>(raycasting_data.cam_pos.x);
    int mapy = static_cast<int>(raycasting_data.cam_pos.y);

    Vector2D delta_dist = {ray_dir.x == 0 ? 1e30 : std::abs(1 / ray_dir.x),
                           ray_dir.y == 0 ? 1e30 : std::abs(1 / ray_dir.y)};

    int step_x;
    int step_y;
    Vector2D side_dist;

    if (ray_dir.x < 0) {
      step_x = -1;
      side_dist.x = (raycasting_data.cam_pos.x - mapx) * delta_dist.x;
    } else {
      step_x = 1;
      side_dist.x = (mapx + 1.0 - raycasting_data.cam_pos.x) * delta_dist.x;
    }
    if (ray_dir.y < 0) {
      step_y = -1;
      side_dist.y = (raycasting_data.cam_pos.y - mapy) * delta_dist.y;
    } else {
      step_y = 1;
      side_dist.y = (mapy + 1.0 - raycasting_data.cam_pos.y) * delta_dist.y;
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

    double perp_wall_dist = 0.0;
    if (side == 0) {
      perp_wall_dist = side_dist.x - delta_dist.x;
    } else {
      perp_wall_dist = side_dist.y - delta_dist.y;
    }

    perp_wall_dist = std::max(perp_wall_dist, 0.001);

    int line_height = static_cast<int>(window_size.y / perp_wall_dist);
    int draw_start = (-line_height / 2) + static_cast<int>(window_size.y / 2);
    draw_start = std::max(draw_start, 0);
    int draw_end = (line_height / 2) + static_cast<int>(window_size.y / 2);
    draw_end = std::min(draw_end, static_cast<int>(window_size.y) - 1);

    int wall_value = 0;
    if (mapy >= 0 && mapy < static_cast<int>(map_data.size()) &&
        mapx >= 0 && mapx < static_cast<int>(map_data[0].size())) {
      wall_value = map_data[mapy][mapx];
    }

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
    float screen_x = static_cast<float>(ray_idx) * window_size.x / raycasting_data.nb_rays;
    float next_screen_x = static_cast<float>(ray_idx + 1) * window_size.x / raycasting_data.nb_rays;

    sf::Color col(color.r, color.g, color.b, color.a);
    float y_start = static_cast<float>(draw_start);
    float y_end = static_cast<float>(draw_end);

    this->_triangle_vertices.append(sf::Vertex(sf::Vector2f(screen_x, y_start), col));
    this->_triangle_vertices.append(sf::Vertex(sf::Vector2f(next_screen_x, y_start), col));
    this->_triangle_vertices.append(sf::Vertex(sf::Vector2f(screen_x, y_end), col));
    this->_triangle_vertices.append(sf::Vertex(sf::Vector2f(next_screen_x, y_start), col));
    this->_triangle_vertices.append(sf::Vertex(sf::Vector2f(next_screen_x, y_end), col));
    this->_triangle_vertices.append(sf::Vertex(sf::Vector2f(screen_x, y_end), col));
  }
  this->_window.draw(this->_triangle_vertices);
  this->_triangle_vertices.clear();
}

void SFMLRenderer::render_basic_map(Registry& r,
                                    std::vector<DrawableItem>& all_drawables,
                                    float min_dimension,
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

  double angle_step = fov / nb_rays;
  const int tile_size = 64;
  _circle.setRadius(tile_size / 3);
  _circle.setFillColor(sf::Color::Red);
  _circle.setOrigin({_circle.getRadius(), _circle.getRadius()});
  _circle.setPosition({static_cast<float>(cam_pos.x * tile_size),
                       static_cast<float>(cam_pos.y * tile_size)});

  for (auto&& [draw, basic_map] : Zipper<Drawable, BasicMap>(r)) {
    if (!draw.enabled) {
      continue;
    }
    RaycastingData raycasting_data {
        .cam_pos = cam_pos,
        .map_size = {static_cast<double>(basic_map.data[0].size()),
                     static_cast<double>(basic_map.data.size())},
        .cam_angle = cam_angle,
        .fov = fov,
        .angle_step = angle_step,
        .nb_rays = nb_rays};
    this->cast_rays(r, raycasting_data, basic_map.data, window_size);
  }
}
