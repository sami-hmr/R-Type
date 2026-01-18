# Entity Loader JSON Format

The EntityLoader system loads game entities, scenes, and plugins from JSON configuration files. This document describes the supported JSON format.

## File Structure

JSON files can use either the new format with top-level sections or the legacy format (direct scene definition).

### New Format

```json
{
  "entities_template": [...],
  "scenes": [...],
  "configs": [...]
}
```

### Legacy Format

Direct scene definition without wrapper:

```json
{
  "name": "scene_name",
  "state": "active",
  "plugins": [...],
  "entities": [...]
}
```

## Top-Level Sections

### entities_template

Defines reusable entity templates that can be instantiated with custom parameters.

```json
"entities_template": [
  {
    "name": "template_name",
    "default_parameters": {
      "param_name": "default_value"
    },
    "components": {
      "plugin:ComponentName": { ... }
    }
  }
]
```

- **name**: Unique identifier for the template
- **default_parameters**: Default values for template parameters (optional)
- **components**: Component definitions with plugin-qualified names

### scenes

Defines game scenes with their plugins and entities.

```json
"scenes": [
  {
    "name": "scene_name",
    "state": "active|disabled",
    "plugins": [...],
    "entities": [...]
  }
]
```

- **name**: Scene identifier (defaults to "default" if omitted)
- **state**: Scene state - "active" or "disabled" (defaults to "disabled")
- **plugins**: Array of plugin configurations
- **entities**: Array of entity definitions

### configs

References to additional configuration directories to load.

```json
"configs": [
  "path/to/config/directory"
]
```

## Plugin Configuration

Plugins are loaded per-scene and provide component systems.

```json
"plugins": [
  {
    "name": "plugin_name",
    "config": {
      "option": "value"
    }
  }
]
```

- **name**: Plugin identifier
- **config**: Plugin-specific configuration (optional)

## Entity Definitions

### Direct Component Definition

```json
{
  "plugin:ComponentName": {
    "property": "value"
  }
}
```

Components are specified using the format `plugin:ComponentName` where:
- **plugin**: Name of the plugin providing the component
- **ComponentName**: Component type within the plugin

### Template-Based Definition

```json
{
  "template": "template_name",
  "parameters": {
    "param": "value"
  },
  "config": {
    "plugin:AdditionalComponent": { ... }
  }
}
```

- **template**: Name of the template to use
- **parameters**: Values to override default parameters
- **config**: Additional components to add (optional)

Templates can also nest other templates by including a template reference in their components.

## Component Properties

### Parameter Substitution

Templates support parameter substitution using the `$` prefix:

```json
"components": {
  "ui:AnimatedSprite": {
    "texture": "$texture_path",
    "framerate": "$fps"
  }
}
```

### Value References

Component properties can reference other component values using hooks:

- `@self`: Reference to the current entity
- `#self:hook_name:property`: Reference to a property on the current entity
- `#global:hook_name:property`: Reference to a global hook value

Example:

```json
"ath:Bar": {
  "max_value": "#self:player_health:max",
  "current_value": "#self:player_health:current"
}
```

### Hooks

Components can define hooks for value storage and retrieval:

```json
"moving:IdStorage": {
  "id": "@self",
  "hook": "player_id",
  "global_hook": true
}
```

- **hook**: Named reference for this value
- **global_hook**: Whether the hook is globally accessible

## Loading Process

1. Files are loaded recursively from specified directories
2. JSON files are processed in alphabetical order
3. Templates are registered first
4. Scenes are created with specified state
5. Plugins are loaded per-scene
6. Entities are spawned with their components

## Error Handling

The loader validates:
- Required fields in component definitions
- Plugin availability before component creation
- Template existence before instantiation
- Value types match expected formats

Errors are logged to stderr but do not halt the loading process.
