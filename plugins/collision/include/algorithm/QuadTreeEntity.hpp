#pragma once

#include "Rect.hpp"
#include "ecs/Registery.hpp"

struct QuadTreeEntity
{
  Registery::Entity id;
  Rect bounds;
};
