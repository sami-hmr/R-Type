# Entity Templates

Templates provide a reusable way to define entity configurations with customizable parameters. They enable efficient entity creation with consistent component structures while allowing variation through parameter substitution.

## Template Definition

Templates are defined in the `entities_template` array in JSON configuration files.

```json
{
  "entities_template": [
    {
      "name": "template_name",
      "default_parameters": {
        "parameter_name": "default_value"
      },
      "components": {
        "plugin:Component": {
          "property": "$parameter_name"
        }
      }
    }
  ]
}
```

### Required Fields

- **name**: Unique identifier for the template
- **components**: Object containing component definitions

### Optional Fields

- **default_parameters**: Default values for template parameters

## Using Templates

### Basic Usage

Reference a template in an entity definition:

```json
{
  "template": "player",
  "parameters": {
    "health_max": 100.0,
    "speed": {"x": 1.0, "y": 1.0}
  }
}
```

### With Additional Components

Add extra components beyond the template definition:

```json
{
  "template": "enemy",
  "parameters": {
    "health_max": 50.0
  },
  "config": {
    "weapon:BasicWeapon": {
      "bullet_type": "fire_ball",
      "cooldown": 2.0
    }
  }
}
```

- **template**: Name of the template to instantiate
- **parameters**: Values to override defaults
- **config**: Additional components not in the template

## Parameter Substitution

Parameters are substituted using the `$` prefix in component definitions.

### Simple Values

```json
"default_parameters": {
  "health": 100.0,
  "team": "player"
},
"components": {
  "life:Health": {
    "current": "$health",
    "max": "$health"
  },
  "life:Team": {
    "name": "$team"
  }
}
```

### Complex Objects

Parameters can be objects or nested structures:

```json
"default_parameters": {
  "speed": {"x": 1.0, "y": 1.0},
  "frame_size": {"x": 128.0, "y": 128.0}
},
"components": {
  "moving:Speed": {
    "speed": "$speed"
  },
  "ui:Sprite": {
    "frame_size": "$frame_size"
  }
}
```

## Nested Templates

Templates can reference other templates, creating a hierarchy.

```json
{
  "name": "base_enemy",
  "default_parameters": {
    "health_max": 10.0,
    "speed": {"x": 0.3, "y": 0.3}
  },
  "components": {
    "life:Health": {
      "current": "$health_max",
      "max": "$health_max"
    },
    "moving:Speed": {
      "speed": "$speed"
    }
  }
}
```

```json
{
  "name": "turret_enemy",
  "default_parameters": {
    "bullet_type": "fire_ball"
  },
  "components": {
    "template": "base_enemy",
    "parameters": {
      "health_max": 20.0,
      "speed": {"x": 0.0, "y": 0.0}
    },
    "config": {
      "weapon:BasicWeapon": {
        "bullet_type": "$bullet_type",
        "cooldown": 1.5
      }
    }
  }
}
```

## Best Practices

### Use Default Parameters

Define sensible defaults for all parameters to minimize configuration:

```json
"default_parameters": {
  "health_max": 10.0,
  "damage_amount": 1,
  "team_name": "enemy",
  "collision_type": "push"
}
```

### Organize by Functionality

Create base templates for common entity types:

- `base_enemy`: Core enemy components
- `base_projectile`: Bullet/projectile components
- `base_ui_element`: UI component foundations

### Parameter Naming

Use descriptive, hierarchical parameter names:

```json
"idle_texture": "path/to/texture.png",
"idle_frame_size": {"x": 64.0, "y": 64.0},
"idle_nb_frames": 8,
"death_texture": "path/to/death.png",
"death_frame_size": {"x": 128.0, "y": 128.0},
"death_nb_frames": 12
```

### Combine Fixed and Variable Components

Keep common components fixed in the template, expose variable ones through parameters:

```json
"components": {
  "ui:Drawable": {},
  "moving:Direction": {
    "direction": {"x": 0.0, "y": 0.0}
  },
  "ui:AnimatedSprite": {
    "texture": "$texture_path",
    "frame_size": "$frame_size"
  }
}
```

## Common Patterns

### Character Template

```json
{
  "name": "character",
  "default_parameters": {
    "health_max": 100.0,
    "speed": {"x": 1.0, "y": 1.0},
    "sprite_size": {"width": "15.0%", "height": "15.0%"},
    "team_name": "player"
  },
  "components": {
    "moving:Position": {
      "pos": {"x": 0.0, "y": 0.0}
    },
    "moving:Speed": {
      "speed": "$speed"
    },
    "life:Health": {
      "current": "$health_max",
      "max": "$health_max"
    },
    "life:Team": {
      "name": "$team_name"
    },
    "ui:Drawable": {}
  }
}
```

### Animated Entity Template

```json
{
  "name": "animated_entity",
  "default_parameters": {
    "idle_texture": "assets/default.png",
    "idle_frame_size": {"x": 64.0, "y": 64.0},
    "idle_nb_frames": 1,
    "idle_framerate": 10.0
  },
  "components": {
    "ui:AnimatedSprite": {
      "animations": [
        {
          "name": "idle",
          "texture": "$idle_texture",
          "frame_size": "$idle_frame_size",
          "nb_frames": "$idle_nb_frames",
          "framerate": "$idle_framerate",
          "loop": true
        }
      ],
      "default_animation": "idle"
    }
  }
}
```

## Template Instantiation

Templates are instantiated either declaratively in JSON or programmatically via code.

### Declarative (JSON)

```json
"entities": [
  {
    "template": "enemy",
    "parameters": {
      "health_max": 25.0
    }
  }
]
```

### Programmatic (C++)

```cpp
auto entity = entity_loader.load_entity_template(
    "enemy",
    additional_components,
    parameters
);
```

## Parameter Resolution

When instantiating a template:

1. Start with default_parameters from the template
2. Override with provided parameters
3. Substitute all `$parameter_name` references in components
4. Apply any additional components from config section
5. Create the entity with final component set

Missing parameters that are referenced will cause an error.
