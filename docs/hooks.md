# Hooks System

The hooks system provides data binding and runtime introspection between components in JSON configurations.

## Hook Types

### Static Hooks (%)

Static hooks read a value once during initialization.

Format: `"%scope:Component:field"`

```json
{
  "speed": "%global:Config:playerSpeed"
}
```

Use case: Reading configuration or template values.

### Dynamic Hooks (#)

Dynamic hooks create live data bindings that auto-update.

Format: `"#scope:Component:field"`

```json
{
  "max_value": "#self:player_health:max"
}
```

Use case: Real-time value synchronization (e.g., health bars tracking health).

### Component Registration Hooks

Register components by name for global or local access.

```json
{
  "moving:IdStorage": {
    "id": "@self",
    "hook": "player_id",
    "global_hook": true
  }
}
```

Use case: Making components accessible across the system.

## Hook Scopes

- `self` - References a component on the same entity
- `global` - References a globally registered component

## Special Syntax

- `@self` - Returns the entity ID itself
- `$parameter` - Template parameter substitution

## Implementation

### C++ Side

Components expose fields using the `HOOKABLE()` macro:

```cpp
struct Health {
  HOOKABLE(int, current);
  HOOKABLE(int, max);
};
```

### JSON Side

Reference exposed fields using hook syntax:

```json
{
  "life:Health": {
    "hook": "player_health",
    "current": 100.0,
    "max": 100.0
  },
  "ath:Bar": {
    "max_value": "#self:player_health:max",
    "current_value": "#self:player_health:current"
  }
}
```

## Resolution Process

1. JSON configurations use hook syntax (`#` or `%`)
2. Plugin initialization reads `hook` and `global_hook` properties via `COMP_INIT` macro
3. Registry resolves references using `get_value()`, `get_ref()`, or `get_value_copy()`
4. Dynamic bindings automatically sync values when sources change
