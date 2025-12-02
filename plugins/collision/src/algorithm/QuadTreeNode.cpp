#include "algorithm/QuadTreeNode.hpp"

void QuadTreeNode::clear()
{
  _entities.clear();

  for (auto & node : _nodes) {
    node.clear();
  }
  _nodes.clear();
}

void QuadTreeNode::split()
{
  double sub_width = bounds.width / 2.0;
  double sub_height = bounds.height / 2.0;

  _nodes.reserve(4);

  _nodes.emplace_back(
      _level + 1, 
      Rect{.x=bounds.x, .y=bounds.y, .width=sub_width, .height=sub_height});

  _nodes.emplace_back(
      _level + 1, 
      Rect{.x=bounds.x + sub_width, .y=bounds.y, .width=sub_width, .height=sub_height});

  _nodes.emplace_back(
      _level + 1, 
      Rect{.x=bounds.x, .y=bounds.y + sub_height, .width=sub_width, .height=sub_height});

  _nodes.emplace_back(
      _level + 1, 
      Rect{.x=bounds.x + sub_width, .y=bounds.y + sub_height, .width=sub_width, .height=sub_height});
}

int QuadTreeNode::get_index(Rect const& rect) const
{
  int index = -1;

  double vertical_midpoint = bounds.x + (bounds.width / 2.0);
  double horizontal_midpoint = bounds.y + (bounds.height / 2.0);

  bool top_half = (rect.y < horizontal_midpoint && 
                   rect.y + rect.height < horizontal_midpoint);

  bool bottom_half = (rect.y > horizontal_midpoint);

  if (rect.x < vertical_midpoint && 
      rect.x + rect.width < vertical_midpoint) {
    if (top_half) {
      index = 0;
    } else if (bottom_half) {
      index = 2;
    }
  }
  else if (rect.x > vertical_midpoint) {
    if (top_half) {
      index = 1;
    } else if (bottom_half) {
      index = 3;
    }
  }

  return index;
}

void QuadTreeNode::insert(ICollisionAlgorithm::CollisionEntity const& entity)
{
    if (!_nodes.empty()) {
        int index = get_index(entity.bounds);
        if (index != -1) {
            _nodes[index].insert(entity);
            return;
        }
    }

    _entities.push_back(entity);
    if (_entities.size() > max_entities && _level < max_levels) {
        if (_nodes.empty()) {
            split();
        }
        auto it = _entities.begin();
        while (it != _entities.end()) {
            int index = get_index(it->bounds);
            if (index != -1) {
                _nodes[index].insert(*it);
                it = _entities.erase(it);
            } else {
                ++it;
            }
        }
    }
}

std::vector<ICollisionAlgorithm::CollisionEntity> QuadTreeNode::retrieve(
    std::vector<ICollisionAlgorithm::CollisionEntity>& return_entities,
    Rect const& rect) const
{
    int index = get_index(rect);

    if (index != -1 && !_nodes.empty()) {
        _nodes[index].retrieve(return_entities, rect);
    }

    return_entities.insert(
        return_entities.end(),
        _entities.begin(),
        _entities.end()
    );

    if (index == -1 && !_nodes.empty()) {
        for (const auto & node : _nodes) {
            node.retrieve(return_entities, rect);
        }
    }  
    return return_entities;
}
