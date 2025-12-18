#pragma once

/**
 * @file Rect.hpp
 * @brief Axis-aligned bounding box for collision detection
 */

/**
 * @struct Rect
 * @brief Center-based rectangle with collision detection methods
 *
 * Represents an axis-aligned rectangle using center point (x, y) and dimensions
 * (width, height). Provides intersection and point containment tests for
 * collision detection and spatial queries.
 *
 * @note Coordinates use center point, not top-left corner
 * @note Used primarily by collision systems and interaction zones
 *
 * @see Collidable, InteractionZone components
 */
struct Rect
{
  double x;  ///< Center X coordinate
  double y;  ///< Center Y coordinate
  double width;  ///< Total width
  double height;  ///< Total height

  /**
   * @brief Tests if this rectangle intersects another
   * @param other Rectangle to test against
   * @return true if rectangles overlap, false otherwise
   *
   * Uses AABB (Axis-Aligned Bounding Box) intersection algorithm.
   * Rectangles are treated as centered at (x, y).
   */
  bool intersects(Rect const& other) const
  {
    double left = x - (width / 2);
    double right = x + (width / 2);
    double top = y - (height / 2);
    double bottom = y + (height / 2);

    double other_left = other.x - (other.width / 2);
    double other_right = other.x + (other.width / 2);
    double other_top = other.y - (other.height / 2);
    double other_bottom = other.y + (other.height / 2);

    return right >= other_left && other_right >= left && bottom >= other_top
        && other_bottom >= top;
  }

  /**
   * @brief Tests if a point lies within this rectangle
   * @param px Point X coordinate
   * @param py Point Y coordinate
   * @return true if point is inside rectangle (inclusive lower bound, exclusive
   * upper)
   */
  bool contains(double px, double py) const
  {
    double left = x - (width / 2);
    double right = x + (width / 2);
    double top = y - (height / 2);
    double bottom = y + (height / 2);

    return px >= left && px < right && py >= top && py < bottom;
  }
};
