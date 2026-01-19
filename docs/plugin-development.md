# Plugin Development Guide

This guide walks you through creating a custom plugin for the R-Type engine using the `moving` plugin as a reference example.

## Plugin Structure

A typical plugin consists of:

```
plugins/your_plugin/
├── CMakeLists.txt          # Build configuration
├── include/
│   └── YourPlugin.hpp      # Plugin class header
└── src/
    └── YourPlugin.cpp      # Plugin implementation
```

## Step 1: Create CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.14)

set(LIB_NAME your_plugin)

add_library(${LIB_NAME} SHARED
    src/YourPlugin.cpp
)

target_compile_features(${LIB_NAME} PRIVATE cxx_std_23)

# Link against the core library and any dependencies
target_link_libraries(${LIB_NAME} ${CORE_LIB} Vector2D)

target_include_directories(${LIB_NAME}
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        include
)
```

## Step 2: Define Your Components

Components are simple data structures. Create them in `include/plugin/components/`:

```cpp
#pragma once

#include "ByteParser/ByteParser.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"

struct Position
{
  Position() = default;

  Position(Vector2D pos, int z = 1)
      : pos(pos)
      , z(z)
  {
  }

  // Serialization support for networking
  DEFAULT_BYTE_CONSTRUCTOR(Position,
                           ([](Vector2D pos, int z = 1)
                            { return Position {pos, z}; }),
                           parseVector2D(),
                           parseByte<int>())
  DEFAULT_SERIALIZE(vector2DToByte(this->pos), type_to_byte(this->z))

  CHANGE_ENTITY_DEFAULT

  // Component data
  Vector2D pos;
  int z;
  bool applied_offset = false;
  
  // Hook system for JSON references
  HOOKABLE(Position, HOOK(pos), HOOK(pos.x), HOOK(pos.y), HOOK(z))
};
```

## Step 3: Create Plugin Header

```cpp
#pragma once

#include "Json/JsonParser.hpp"
#include "ecs/EventManager.hpp"
#include "ecs/Registry.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"

class Moving : public APlugin
{
public:
  Moving(Registry& r, EventManager& em, EntityLoader& l);

private:
  // Component initializers - called from JSON
  void init_pos(Ecs::Entity const& entity, JsonObject& obj);
  void init_speed(Ecs::Entity const& entity, JsonObject& obj);
  
  // Systems - game logic that runs every frame
  void moving_system(Registry&);
};
```

## Step 4: Implement Plugin Class

```cpp
#include "Moving.hpp"
#include "ecs/InitComponent.hpp"
#include "ecs/zipper/ZipperIndex.hpp"
#include "plugin/components/Position.hpp"
#include "plugin/components/Speed.hpp"

Moving::Moving(Registry& r, EventManager& em, EntityLoader& l)
    : APlugin("moving",  // Plugin name (used as namespace in JSON)
              r,
              em,
              l,
              {},  // Dependencies (other plugins this one needs)
              {
                // Map component names to initializer functions
                COMP_INIT(Position, Position, init_pos),
                COMP_INIT(Speed, Speed, init_speed)
              })
{
  // Register components with the registry
  REGISTER_COMPONENT(Position)
  REGISTER_COMPONENT(Speed)
  
  // Add systems (priority controls execution order)
  this->_registry.get().add_system(
      [this](Registry& r) { this->moving_system(r); }, 
      4  // Priority (higher = runs earlier)
  );
  
  // Subscribe to events
  SUBSCRIBE_EVENT(UpdateDirection, {
    if (!this->_registry.get().has_component<Direction>(event.entity)) {
      return false;
    }
    auto& comp = this->_registry.get().get_components<Direction>()[event.entity];
    comp->direction.x = std::max(-1.0, std::min(comp->direction.x + event.x_axis, 1.0));
    comp->direction.y = std::max(-1.0, std::min(comp->direction.y + event.y_axis, 1.0));
  })
}

// Component initializer - parses JSON and creates component
void Moving::init_pos(Ecs::Entity const& entity, JsonObject& obj)
{
  // Extract required value from JSON
  auto pos = get_value<Position, Vector2D>(
      this->_registry.get(), obj, entity, "pos");

  if (!pos.has_value()) {
    std::cerr << "Error: Position component missing required 'pos' field\n";
    return;
  }

  // Extract optional value
  int z = 1;
  if (obj.contains("z")) {
    auto z_value = get_value<Position, int>(
        this->_registry.get(), obj, entity, "z");
    if (z_value) {
      z = z_value.value();
    }
  }
  
  // Create and attach component to entity
  init_component<Position>(this->_registry.get(),
                          this->_event_manager.get(),
                          entity,
                          pos.value(),
                          z);
}

// System - runs every frame on entities with required components
void Moving::moving_system(Registry& reg)
{
  double dt = reg.clock().delta_seconds();

  // Iterate over all entities with Position, Direction, and Speed
  for (auto&& [entity, position, direction, speed] :
       ZipperIndex<Position, Direction, Speed>(reg))
  {
    Vector2D movement = direction.direction.normalize() * speed.speed * dt;
    position.pos += movement;
    
    // Emit component update for networking
    if (movement.length() != 0) {
      this->_event_manager.get().emit<ComponentBuilder>(
          entity, reg.get_component_key<Position>(), position.to_bytes());
    }
  }
}

// Required: Export entry point for dynamic loading
extern "C"
{
PLUGIN_EXPORT void* entry_point(Registry& r, EventManager& em, EntityLoader& e)
{
  return new Moving(r, em, e);
}
}
```

## Step 5: Use Plugin in JSON Configuration

Once your plugin is built, use it in configuration files:

```json
{
  "scenes": [
    {
      "name": "game",
      "state": "active",
      "plugins": [
        {
          "name": "moving",
          "config": {}
        }
      ],
      "entities": [
        {
          "moving:Position": {
            "pos": {"x": 0.5, "y": 0.5},
            "z": 10
          },
          "moving:Speed": {
            "speed": {"x": 2.0, "y": 2.0}
          },
          "moving:Direction": {
            "direction": {"x": 1.0, "y": 0.0}
          }
        }
      ]
    }
  ]
}
```

## Key Concepts

### Component Naming Convention
Components are namespaced with the plugin name: `plugin_name:ComponentName`

Example: `moving:Position`, `collision:Collidable`

### System Priority
Systems execute in priority order (higher numbers first):
- 1000+ : Pre-processing (e.g., apply offsets)
- 100-999: Main logic (e.g., movement, collision)
- 0-99: Post-processing (e.g., remove offsets, rendering)

### The Zipper Pattern
Use `ZipperIndex` to iterate over entities with specific components:

```cpp
for (auto&& [entity_id, comp1, comp2] : ZipperIndex<Component1, Component2>(reg))
{
  // Only entities with BOTH components are processed
  comp1.value += comp2.delta;
}
```

### Hook System
Hooks allow JSON values to reference component fields from other entities:

```json
{
  "moving:Position": {
    "pos": {"x": 0.5, "y": 0.5},
    "hook": "player_position",
    "global_hook": true
  },
  "ui:Text": {
    "text": "#global:player_position:pos.x"
  }
}
```

### Helper Macros

- `REGISTER_COMPONENT(Type)` - Register component with registry
- `COMP_INIT(name, type, method)` - Map JSON component name to initializer
- `SUBSCRIBE_EVENT(EventType, {...})` - Handle events
- `DEFAULT_BYTE_CONSTRUCTOR(...)` - Enable network serialization
- `HOOKABLE(Type, ...)` - Enable hook system for component fields

## Building Your Plugin

Add your plugin to the main CMakeLists.txt:

```cmake
add_subdirectory(plugins/your_plugin)
```

Then rebuild:

```bash
cmake --build build
```

Your plugin will be compiled to `build/plugins/your_plugin.so` (or `.dll` on Windows).

## Common Patterns

### Reading JSON Values
```cpp
// Required field
auto value = get_value<MyComponent, Type>(registry, json_obj, entity, "field_name");
if (!value.has_value()) {
  std::cerr << "Missing required field\n";
  return;
}

// Optional field with default
Type field = default_value;
if (json_obj.contains("field_name")) {
  auto opt = get_value<MyComponent, Type>(registry, json_obj, entity, "field_name");
  if (opt) field = opt.value();
}
```

### Emitting Events
```cpp
// Simple event
this->_event_manager.get().emit<MyEvent>(param1, param2);

// Component change (for networking)
this->_event_manager.get().emit<ComponentBuilder>(
    entity,
    registry.get_component_key<MyComponent>(),
    component.to_bytes()
);
```

### Checking Component Existence
```cpp
if (registry.has_component<MyComponent>(entity)) {
  auto& comp = registry.get_components<MyComponent>()[entity];
  // Use component
}
```

## Next Steps

- See [COMPONENTS.md](COMPONENTS.md) for all available components
- See [EVENTS.md](EVENTS.md) for event system details
- Look at existing plugins in `plugins/` for more examples
- Check [hooks.md](hooks.md) for advanced hook patterns
