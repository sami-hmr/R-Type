# Component Reference

This document describes all available components in the R-Type ECS system and how to use them in JSON configuration files.

## Table of Contents

- [Movement Components](#movement-components)
- [UI and Rendering Components](#ui-and-rendering-components)
- [Life and Combat Components](#life-and-combat-components)
- [Weapon Components](#weapon-components)
- [Collision Components](#collision-components)
- [Projectile Components](#projectile-components)
- [AI Components](#ai-components)
- [UI Interactive Components](#ui-interactive-components)
- [Audio Components](#audio-components)
- [Input and Controller Components](#input-and-controller-components)
- [Game Logic Components](#game-logic-components)
- [Special Mechanics Components](#special-mechanics-components)

---

## Movement Components

### moving:Position

Position of an entity in 2D space with z-layer for rendering order.

**Fields:**
- `pos` (Vector2D): x and y coordinates
- `z` (int, optional): z-layer for rendering order (default: 1)
- `hook` (string, optional): hook name for referencing this position

**Example:**
```json
"moving:Position": {
  "pos": {"x": 0.5, "y": 0.5},
  "z": 2
}
```

### moving:Speed

Movement speed in x and y directions.

**Fields:**
- `speed` (Vector2D): x and y speed values

**Example:**
```json
"moving:Speed": {
  "speed": {"x": 1.0, "y": 1.0}
}
```

### moving:Direction

Current movement direction vector.

**Fields:**
- `direction` (Vector2D): x and y direction components (normalized during movement)

**Example:**
```json
"moving:Direction": {
  "direction": {"x": 1.0, "y": 0.0}
}
```

### moving:Facing

Direction the entity is facing (for sprite orientation).

**Fields:**
- `direction` (Vector2D): facing direction
- `plane` (bool, optional): whether to flip sprite based on direction (default: false)

**Example:**
```json
"moving:Facing": {
  "direction": {"x": 1.0, "y": 0.0},
  "plane": true
}
```

### moving:Offset

Position offset from parent entity or base position.

**Fields:**
- `offset` (Vector2D): x and y offset values

**Example:**
```json
"moving:Offset": {
  "offset": {"x": 0.0, "y": -0.1}
}
```

### moving:IdStorage

Store and reference entity IDs through hooks.

**Fields:**
- `id` (string): entity ID to store (use "@self" for current entity)
- `hook` (string): hook name for referencing this ID
- `global_hook` (bool, optional): whether hook is globally accessible
- `context` (string, optional): context for the ID storage

**Example:**
```json
"moving:IdStorage": {
  "id": "@self",
  "hook": "player_id",
  "global_hook": true
}
```

---

## UI and Rendering Components

### ui:Drawable

Marks an entity as drawable and visible.

**Fields:**
- `enabled` (bool, optional): whether entity is visible (default: true)
- `stretch` (bool, optional): whether to stretch the sprite (default: false)

Hidden feild: true_size, Vector2D hookable in case of stretching

**Example:**
```json
"ui:Drawable": {}
```

```json
"ui:Drawable": {
  "enabled": true,
  "stretch": false
}
```

### ui:Sprite

Static sprite with texture and scale.

**Fields:**
- `texture` (string): path to texture file
- `size` (Vector2D, optional): width and height (percentage strings like "10.0%" or floats)

**Example:**
```json
"ui:Sprite": {
  "texture": "assets/ship.png",
  "size": {"width": "15.0%", "height": "15.0%"}
}
```

### ui:AnimatedSprite

Animated sprite with multiple animation states.

**Fields:**
- `animations` (array): list of animation definitions
  - `name` (string): animation identifier
  - `texture` (string): texture path
  - `frame_size` (Vector2D): size of each frame
  - `frame_pos` (Vector2D): starting position in spritesheet
  - `direction` (Vector2D): sprite direction
  - `sprite_size` (Vector2D): display size (percentage or float)
  - `framerate` (double): frames per second
  - `nb_frames` (int): number of frames
  - `loop` (bool): whether animation loops
  - `rollback` (bool): play animation backward after completion
- `default_animation` (string): name of default animation

**Example:**
```json
"ui:AnimatedSprite": {
  "animations": [
    {
      "name": "idle",
      "texture": "assets/player.png",
      "frame_size": {"x": 350.0, "y": 150.0},
      "frame_pos": {"x": 0.0, "y": 0.0},
      "direction": {"x": 1.0, "y": 0.0},
      "sprite_size": {"width": "15.0%", "height": "15.0%"},
      "framerate": 10.0,
      "nb_frames": 7,
      "loop": true,
      "rollback": true
    }
  ],
  "default_animation": "idle"
}
```

### ui:Text

Text rendering component.

**Fields:**
- `font` (string): path to font file
- `text` (string): text content
- `size` (Vector2D, optional): text size
- `fill_color` (Color): text color
- `outline_color` (Color): outline color
- `outline` (bool): whether to draw outline
- `outline_thickness` (double): thickness of outline
- `placeholder` (string, optional): placeholder text

**Example:**
```json
"ui:Text": {
  "font": "assets/font.ttf",
  "text": "Score: 0",
  "size": {"width": "5.0%", "height": "5.0%"},
  "fill_color": {"r": 255, "g": 255, "b": 255, "a": 255},
  "outline_color": {"r": 0, "g": 0, "b": 0, "a": 255},
  "outline": true,
  "outline_thickness": 2.0
}
```

### ui:Background

Background layer rendering.

**Fields:**
- `layers` (array): list of background texture paths

**Example:**
```json
"ui:Background": {
  "layers": ["assets/bg1.png", "assets/bg2.png"]
}
```

### ui:Camera

Camera viewport component.

**Example:**
```json
"ui:Camera": {}
```

---

## Life and Combat Components

### life:Health

Health system with current and maximum values.

**Fields:**
- `current` (double): current health
- `max` (double): maximum health
- `hook` (string, optional): hook name for health value

**Example:**
```json
"life:Health": {
  "hook": "player_health",
  "current": 100.0,
  "max": 100.0
}
```

### life:Damage

Damage amount dealt on collision.

**Fields:**
- `amount` (int): damage value

**Example:**
```json
"life:Damage": {
  "amount": 10
}
```

### life:Heal

Healing amount provided on collision.

**Fields:**
- `amount` (int): heal value

**Example:**
```json
"life:Heal": {
  "amount": 20
}
```

### life:Team

Team affiliation for determining friend or foe.

**Fields:**
- `name` (string): team identifier (e.g., "player", "enemy")

**Example:**
```json
"life:Team": {
  "name": "player"
}
```

---

## Weapon Components

### weapon:BasicWeapon

Standard shooting weapon with cooldown and magazine system.

**Fields:**
- `bullet_type` (string): entity template name for bullets
- `magazine_size` (int): rounds per magazine
- `magazine_nb` (int): number of magazines
- `reload_time` (double): seconds to reload
- `cooldown` (double): seconds between shots
- `offset_x` (double, optional): x offset for bullet spawn (default: 0.0)
- `offset_y` (double, optional): y offset for bullet spawn (default: 0.0)
- `attack_animation` (string, optional): animation to play when firing

**Example:**
```json
"weapon:BasicWeapon": {
  "bullet_type": "fire_bullet",
  "magazine_size": 10,
  "magazine_nb": 5,
  "reload_time": 2.0,
  "cooldown": 0.5,
  "attack_animation": "attack"
}
```

### weapon:ChargeWeapon

Chargeable weapon with power scaling based on charge time.

**Fields:**
- All fields from BasicWeapon, plus:
- `charge_time` (double): maximum charge time in seconds
- `max_scale` (double): maximum scale multiplier at full charge
- `min_charge_threshold` (double): minimum charge level to fire (0.0-1.0)
- `scale_damage` (bool): whether damage scales with charge level
- `charge_indicator` (string, optional): entity template for charge indicator

**Example:**
```json
"weapon:ChargeWeapon": {
  "bullet_type": "beam_segment",
  "magazine_size": 10,
  "magazine_nb": 3,
  "reload_time": 2.0,
  "cooldown": 0.5,
  "charge_time": 2.0,
  "max_scale": 3.0,
  "min_charge_threshold": 0.3,
  "scale_damage": true,
  "attack_animation": "attack",
  "charge_indicator": "charge_effect"
}
```

### weapon:DelayedWeapon

Weapon with delayed firing after trigger.

**Fields:**
- All fields from BasicWeapon, plus:
- `delay` (double): delay in seconds before firing

**Example:**
```json
"weapon:DelayedWeapon": {
  "bullet_type": "missile",
  "magazine_size": 5,
  "magazine_nb": 3,
  "reload_time": 3.0,
  "cooldown": 1.0,
  "delay": 0.5
}
```

---

## Collision Components

### collision:Collidable

Collision box with type and size.

**Fields:**
- `size` (Vector2D): width and height of collision box
- `collision_type` (string): "solid", "trigger", "push", or "bounce"

**Example:**
```json
"collision:Collidable": {
  "size": {"width": 0.2, "height": 0.2},
  "collision_type": "solid"
}
```

```json
"collision:Collidable": {
  "size": {"width": 0.15, "height": 0.15},
  "collision_type": "trigger"
}
```

### collision:InteractionZone

Circular interaction radius for detecting nearby entities.

**Fields:**
- `radius` (double): interaction radius

**Example:**
```json
"collision:InteractionZone": {
  "radius": 2.0
}
```

### collision:InteractionBorders

Border interaction handling for entities.

**Example:**
```json
"collision:InteractionBorders": {}
```

---

## Projectile Components

### projectile:Temporal

Lifetime-based destruction (entity destroyed after time).

**Fields:**
- `lifetime` (double): seconds before entity is destroyed

**Example:**
```json
"projectile:Temporal": {
  "lifetime": 3.0
}
```

### projectile:Fragile

Hit-count based destruction (entity destroyed after N hits).

**Fields:**
- `hits` (int): number of hits before destruction (0 = destroyed on first hit)

**Example:**
```json
"projectile:Fragile": {
  "hits": 0
}
```

```json
"projectile:Fragile": {
  "hits": 3
}
```

---

## AI Components

### ai:MovementBehavior

Enemy movement patterns.

**Fields:**
- `movement_type` (string): "straight", "turret", "follow", "circle", "wave", "zigzag", "glue"
- `params` (object, optional): parameters specific to movement type

**Example:**
```json
"ai:MovementBehavior": {
  "movement_type": "wave"
}
```

```json
"ai:MovementBehavior": {
  "movement_type": "circle",
  "params": {
    "radius": 2.0,
    "speed": 1.0
  }
}
```

### ai:AttackBehavior

Enemy attack patterns.

**Fields:**
- `attack_type` (string): "continuous", "burst", etc.
- `params` (object, optional): attack-specific parameters

**Example:**
```json
"ai:AttackBehavior": {
  "attack_type": "continuous"
}
```

---

## UI Interactive Components

### ath:Button

Interactive button with states.

**Fields:**
- `state` (string): current button state
- `hover_state` (string, optional): state when hovered
- `pressed_state` (string, optional): state when pressed

**Example:**
```json
"ath:Button": {
  "state": "normal",
  "hover_state": "hover",
  "pressed_state": "pressed"
}
```

### ath:Clickable

Click event handling.

**Example:**
```json
"ath:Clickable": {}
```

### ath:Bar

Progress or health bar display.

**Fields:**
- `size` (Vector2D): bar dimensions
- `max_value` (double or hook): maximum value
- `current_value` (double or hook): current value
- `offset` (Vector2D, optional): offset from entity position
- `color` (Color): bar color
- `outline` (bool, optional): whether to draw outline
- `texture_path` (string, optional): texture for bar

**Example:**
```json
"ath:Bar": {
  "size": {"x": "10.0%", "y": "1.0%"},
  "max_value": "#self:player_health:max",
  "current_value": "#self:player_health:current",
  "offset": {"x": 0.0, "y": -0.1},
  "color": {"r": 100, "g": 255, "b": 100, "a": 255},
  "outline": true
}
```

### ath:Slider

Slider UI component.

**Fields:**
- `min_value` (double): minimum slider value
- `max_value` (double): maximum slider value
- `current_value` (double): current slider value

**Example:**
```json
"ath:Slider": {
  "min_value": 0.0,
  "max_value": 100.0,
  "current_value": 50.0
}
```

---

## Audio Components

### sound:SoundManager

Sound effect management.

**Fields:**
- `sounds` (object): map of sound names to file paths

**Example:**
```json
"sound:SoundManager": {
  "sounds": {
    "shoot": "assets/shoot.wav",
    "explosion": "assets/explosion.wav"
  }
}
```

### sound:MusicManager

Background music management.

**Fields:**
- `music_path` (string): path to music file
- `loop` (bool): whether music loops

**Example:**
```json
"sound:MusicManager": {
  "music_path": "assets/bgm.ogg",
  "loop": true
}
```

### sound:Volume

Volume control for audio.

**Fields:**
- `master` (double): master volume (0.0-1.0)
- `music` (double): music volume (0.0-1.0)
- `sfx` (double): sound effects volume (0.0-1.0)

**Example:**
```json
"sound:Volume": {
  "master": 0.8,
  "music": 0.6,
  "sfx": 0.7
}
```

---

## Input and Controller Components

### controller:Input

Input field component for text entry.

**Fields:**
- `enabled` (bool, optional): whether input is active
- `buffer` (string, optional): current input text

**Example:**
```json
"controller:Input": {
  "enabled": true,
  "buffer": ""
}
```

### controller:Controllable

Keyboard/controller input mapping.

**Fields:**
- `key_bindings` (object): map of actions to keys

**Example:**
```json
"controller:Controllable": {
  "key_bindings": {
    "move_up": "W",
    "move_down": "S",
    "shoot": "SPACE"
  }
}
```

---

## Game Logic Components

### score:ScoreManager

Score tracking and management.

**Fields:**
- `score` (int): current score value

**Example:**
```json
"score:ScoreManager": {
  "score": 0
}
```

### inventory:Inventory

Item inventory system.

**Fields:**
- `max_items` (int): maximum number of item slots
- `inventory` (array): list of items
  - `item_name` (string): item identifier
  - `nb` (int): quantity
  - `item` (object): item data with events
  - `artefact_template` (string, optional): entity template for item

**Example:**
```json
"inventory:Inventory": {
  "max_items": 4,
  "inventory": [
    {
      "item_name": "heal",
      "nb": 2,
      "item": {
        "on_use": [
          {
            "HealEvent": {
              "entity": "@self",
              "amount": 50
            }
          }
        ]
      },
      "artefact_template": "heal_item"
    }
  ]
}
```

### inventory:Item

Individual item definition.

**Fields:**
- `item_data` (object): item properties and events

**Example:**
```json
"inventory:Item": {
  "item_data": {
    "name": "health_potion",
    "effect": "heal"
  }
}
```

### actions:ActionTrigger

Event triggering system.

**Fields:**
- `actions` (array): list of events to trigger
- `trigger_type` (string): when to trigger ("on_create", "on_destroy", etc.)

**Example:**
```json
"actions:ActionTrigger": {
  "trigger_type": "on_create",
  "actions": [
    {
      "SpawnEntity": {
        "template": "particle_effect"
      }
    }
  ]
}
```

### wave:Wave

Wave/level management.

**Fields:**
- `wave_number` (int): current wave number
- `enemies_remaining` (int): enemies left in wave

**Example:**
```json
"wave:Wave": {
  "wave_number": 1,
  "enemies_remaining": 10
}
```

### wave:WaveTag

Wave identification tag.

**Fields:**
- `wave_id` (int): wave identifier

**Example:**
```json
"wave:WaveTag": {
  "wave_id": 1
}
```

---

## Special Mechanics Components

### target:Follower

Follow target entity behavior.

**Fields:**
- Empty object (target is assigned automatically)

**Example:**
```json
"target:Follower": {}
```

### Parasite

Parasite attachment behavior.

**Fields:**
- `host_entity` (entity): entity to attach to

**Example:**
```json
"Parasite": {
  "host_entity": 0
}
```

### Spawner

Entity spawning system.

**Fields:**
- `spawn_template` (string): entity template to spawn
- `spawn_rate` (double): seconds between spawns
- `max_spawns` (int): maximum number of spawns

**Example:**
```json
"Spawner": {
  "spawn_template": "enemy_fighter",
  "spawn_rate": 3.0,
  "max_spawns": 10
}
```

### Formation

Formation movement patterns.

**Fields:**
- `formation_type` (string): type of formation
- `position_in_formation` (int): position index

**Example:**
```json
"Formation": {
  "formation_type": "v_shape",
  "position_in_formation": 2
}
```

### ScaleModifier

Scale transformation component.

**Fields:**
- `scale` (Vector2D): x and y scale multipliers

**Example:**
```json
"ScaleModifier": {
  "scale": {"x": 2.0, "y": 2.0}
}
```

---

## Using Hooks and References

Components can reference values from other components using hooks:

- `#self:hook_name:field` - Reference a field from current entity's hooked component
- `#global:hook_name:field` - Reference a field from globally hooked component
- `@self` - Reference the current entity ID

**Example:**
```json
"ath:Bar": {
  "max_value": "#self:player_health:max",
  "current_value": "#self:player_health:current"
}
```

## Using Parameters in Templates

Templates support parameters using `$parameter_name` syntax:

```json
"default_parameters": {
  "damage_amount": 5,
  "speed": {"x": 1.5, "y": 1.5}
},
"components": {
  "life:Damage": {
    "amount": "$damage_amount"
  },
  "moving:Speed": {
    "speed": "$speed"
  }
}
```
