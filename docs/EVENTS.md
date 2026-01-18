# Event Reference

This document describes all available events in the R-Type ECS system and how to trigger them from JSON or code.

## Table of Contents

- [Audio Events](#audio-events)
- [Input and IO Events](#input-and-io-events)
- [Combat and Life Events](#combat-and-life-events)
- [Weapon Events](#weapon-events)
- [Movement Events](#movement-events)
- [Camera Events](#camera-events)
- [Animation Events](#animation-events)
- [Scene and Level Events](#scene-and-level-events)
- [Entity Management Events](#entity-management-events)
- [Inventory Events](#inventory-events)
- [Interaction Events](#interaction-events)
- [Network Events](#network-events)
- [System Events](#system-events)

---

## Audio Events

### PlaySoundEvent

Play a sound effect on an entity.

**Fields:**
- `entity` (Entity): entity to play sound on
- `name` (string): sound identifier
- `volume` (double): volume level 0.0-100.0 (default: 100.0)
- `pitch` (double): pitch multiplier (default: 1.0)
- `loop` (bool): whether to loop the sound (default: false)

**Example:**
```json
{
  "PlaySoundEvent": {
    "entity": 0,
    "name": "shoot",
    "volume": 80.0,
    "pitch": 1.0,
    "loop": false
  }
}
```

### StopSoundEvent

Stop a specific sound on an entity.

**Fields:**
- `entity` (Entity): entity to stop sound on
- `name` (string): sound identifier to stop

**Example:**
```json
{
  "StopSoundEvent": {
    "entity": 0,
    "name": "shoot"
  }
}
```

### StopAllSoundsEvent

Stop all currently playing sounds.

**Example:**
```json
{
  "StopAllSoundsEvent": {}
}
```

### PlayMusicEvent

Play background music.

**Fields:**
- `entity` (Entity): entity to play music on
- `name` (string): music identifier
- `volume` (double): volume level 0.0-100.0 (default: 100.0)
- `pitch` (double): pitch multiplier (default: 1.0)
- `loop` (bool): whether to loop the music (default: true)

**Example:**
```json
{
  "PlayMusicEvent": {
    "entity": 0,
    "name": "bgm_level1",
    "volume": 60.0,
    "pitch": 1.0,
    "loop": true
  }
}
```

### StopMusicEvent

Stop a specific music track.

**Fields:**
- `entity` (Entity): entity to stop music on
- `name` (string): music identifier to stop

**Example:**
```json
{
  "StopMusicEvent": {
    "entity": 0,
    "name": "bgm_level1"
  }
}
```

### StopAllMusicsEvent

Stop all playing music.

**Example:**
```json
{
  "StopAllMusicsEvent": {}
}
```

---

## Input and IO Events

### KeyPressedEvent

Triggered when keyboard keys are pressed.

**Fields:**
- `keys` (array of strings): list of pressed keys
- `key_unicode` (string, optional): unicode character if printable

**Example:**
```json
{
  "KeyPressedEvent": {
    "keys": ["W", "SPACE"],
    "key_unicode": "w"
  }
}
```

**Available Keys:**
SHIFT, CTRL, ALT, ENTER, LEFT, RIGHT, DOWN, UP, A-Z, ECHAP, DELETE, SPACE, SLASH, 0-9

### KeyReleasedEvent

Triggered when keyboard keys are released.

**Fields:**
- `keys` (array of strings): list of released keys
- `key_unicode` (string, optional): unicode character if printable

**Example:**
```json
{
  "KeyReleasedEvent": {
    "keys": ["W"]
  }
}
```

### MousePressedEvent

Triggered when mouse buttons are pressed.

**Fields:**
- `position` (Vector2D): mouse position
- `button` (string): "MOUSELEFT", "MOUSERIGHT", or "MOUSEMIDDLE"

**Example:**
```json
{
  "MousePressedEvent": {
    "position": {"x": 0.5, "y": 0.5},
    "button": "MOUSELEFT"
  }
}
```

### MouseReleasedEvent

Triggered when mouse buttons are released.

**Fields:**
- `position` (Vector2D): mouse position
- `button` (string): mouse button released

**Example:**
```json
{
  "MouseReleasedEvent": {
    "position": {"x": 0.5, "y": 0.5},
    "button": "MOUSELEFT"
  }
}
```

### InputFocusEvent

Set input focus to a specific entity.

**Fields:**
- `entity` (Entity): entity to focus

**Example:**
```json
{
  "InputFocusEvent": {
    "entity": 42
  }
}
```

---

## Combat and Life Events

### DamageEvent

Deal damage to an entity.

**Fields:**
- `target` (Entity or hook): entity to damage (can use "@self")
- `source` (Entity): entity dealing damage
- `amount` (int): damage amount

**Example:**
```json
{
  "DamageEvent": {
    "entity": "@self",
    "source": 0,
    "amount": 25
  }
}
```

### HealEvent

Heal an entity.

**Fields:**
- `target` (Entity or hook): entity to heal (can use "@self")
- `amount` (int): heal amount

**Example:**
```json
{
  "HealEvent": {
    "entity": "@self",
    "amount": 50
  }
}
```

### DeathEvent

Triggered when an entity dies.

**Fields:**
- `entity` (Entity): entity that died
- `killer` (Entity): entity that caused the death

**Note:** This event is usually emitted by the system, not manually triggered.

---

## Weapon Events

### FireBullet

Fire a weapon.

**Fields:**
- `entity` (Entity): entity with weapon to fire

**Example:**
```json
{
  "FireBullet": {
    "entity": 0
  }
}
```

### StartChargeWeapon

Begin charging a charge weapon.

**Fields:**
- `entity` (Entity): entity with charge weapon

**Example:**
```json
{
  "StartChargeWeapon": {
    "entity": 0
  }
}
```

### ReleaseChargeWeapon

Release and fire a charged weapon.

**Fields:**
- `entity` (Entity): entity with charge weapon

**Example:**
```json
{
  "ReleaseChargeWeapon": {
    "entity": 0
  }
}
```

### WeaponSwitchEvent

Switch to a different weapon type.

**Fields:**
- `entity` (Entity): entity to switch weapon on
- `weapon_type` (string): type of weapon to switch to
- `params` (object): weapon-specific parameters

**Example:**
```json
{
  "WeaponSwitchEvent": {
    "entity": 0,
    "weapon_type": "basic",
    "params": {
      "bullet_type": "fire_bullet"
    }
  }
}
```

---

## Movement Events

### UpdateDirection

Update entity movement direction.

**Fields:**
- `entity` (Entity): entity to update
- `x_axis` (double): x-axis change (-1.0 to 1.0)
- `y_axis` (double): y-axis change (-1.0 to 1.0)

**Example:**
```json
{
  "UpdateDirection": {
    "entity": 0,
    "x_axis": 1.0,
    "y_axis": 0.0
  }
}
```

### SpeedModifierEvent

Multiply entity speed by a factor.

**Fields:**
- `target` (Entity): entity to modify
- `source` (Entity): entity causing the modification
- `multiplier` (double): speed multiplier (e.g., 0.5 for half speed, 2.0 for double)

**Example:**
```json
{
  "SpeedModifierEvent": {
    "target": 0,
    "source": 1,
    "multiplier": 1.5
  }
}
```

### SpeedSwitcherEvent

Set entity to a specific speed.

**Fields:**
- `target` (Entity): entity to modify
- `source` (Entity): entity causing the change
- `new_speed` (double): new speed value

**Example:**
```json
{
  "SpeedSwitcherEvent": {
    "target": 0,
    "source": 1,
    "new_speed": 2.0
  }
}
```

### CollisionEvent

Triggered when two entities collide.

**Fields:**
- `a` (Entity): first entity
- `b` (Entity): second entity

**Note:** This event is emitted by the collision system.

---

## Camera Events

### CamAggroEvent

Make camera follow a target entity.

**Fields:**
- `entity` (Entity): entity for camera to follow

**Example:**
```json
{
  "CamAggroEvent": {
    "entity": 0
  }
}
```

### CamMoveEvent

Move camera to a specific position.

**Fields:**
- `target` (Vector2D): target position

**Example:**
```json
{
  "CamMoveEvent": {
    "target": {"x": 0.5, "y": 0.5}
  }
}
```

### CamZoomEvent

Zoom the camera.

**Fields:**
- `size` (Vector2D): new camera viewport size

**Example:**
```json
{
  "CamZoomEvent": {
    "size": {"x": 800.0, "y": 600.0}
  }
}
```

### CamRotateEvent

Rotate the camera.

**Fields:**
- `rotation` (double): target rotation angle in degrees
- `speed` (double): rotation speed

**Example:**
```json
{
  "CamRotateEvent": {
    "rotation": 45.0,
    "speed": 1.0
  }
}
```

### CamSpeedEvent

Set camera movement speed.

**Fields:**
- `speed` (Vector2D): camera movement speed

**Example:**
```json
{
  "CamSpeedEvent": {
    "speed": {"x": 1.0, "y": 1.0}
  }
}
```

### CameraShakeEvent

Create a screen shake effect.

**Fields:**
- `trauma` (double): shake intensity (0.0-1.0)
- `angle` (double): maximum shake angle
- `offset` (double): maximum shake offset
- `duration` (double): shake duration in seconds

**Example:**
```json
{
  "CameraShakeEvent": {
    "trauma": 0.8,
    "angle": 10.0,
    "offset": 5.0,
    "duration": 0.5
  }
}
```

---

## Animation Events

### PlayAnimationEvent

Play an animation on an entity.

**Fields:**
- `name` (string): animation name
- `entity` (Entity): entity to play animation on
- `framerate` (double): animation framerate
- `loop` (bool): whether to loop the animation
- `rollback` (bool): play animation backward after completion

**Example:**
```json
{
  "PlayAnimationEvent": {
    "name": "attack",
    "entity": 0,
    "framerate": 10.0,
    "loop": false,
    "rollback": false
  }
}
```

### AnimationStartEvent

Triggered when an animation starts.

**Fields:**
- `name` (string): animation name
- `entity` (Entity): entity playing the animation

**Note:** This event is emitted by the animation system.

### AnimationEndEvent

Triggered when an animation finishes.

**Fields:**
- `name` (string): animation name that ended
- `entity` (Entity): entity that finished the animation

**Note:** This event is emitted by the animation system.

---

## Scene and Level Events

### SceneChangeEvent

Change to a different scene.

**Fields:**
- `target_scene` (string): scene to load
- `reason` (string, optional): reason for scene change
- `force` (bool, optional): force scene change
- `main` (bool, optional): whether this is the main scene

**Example:**
```json
{
  "SceneChangeEvent": {
    "target_scene": "level_2",
    "reason": "completed_level",
    "force": false,
    "main": true
  }
}
```

### DisableSceneEvent

Disable a scene without unloading it.

**Fields:**
- `target_scene` (string): scene to disable

**Example:**
```json
{
  "DisableSceneEvent": {
    "target_scene": "pause_menu"
  }
}
```

### WaveSpawnEvent

Spawn a wave of enemies.

**Fields:**
- `wave_templates` (array of strings): list of entity templates to spawn

**Example:**
```json
{
  "WaveSpawnEvent": {
    "wave_templates": ["enemy_fighter", "enemy_bomber", "enemy_fighter"]
  }
}
```

---

## Entity Management Events

### LoadEntityTemplate

Load and instantiate an entity from a template.

**Fields:**
- `template` (string): entity template name
- `aditionals` (array, optional): additional component data to override

**Example:**
```json
{
  "LoadEntityTemplate": {
    "template": "enemy_fighter"
  }
}
```

**With additional components:**
```json
{
  "LoadEntityTemplate": {
    "template": "bullet",
    "aditionals": [
      ["moving:Position", "..."],
      ["life:Team", "..."]
    ]
  }
}
```

### CreateEntity

Create a new entity with components.

**Fields:**
- `additionals` (array): component data for the new entity

**Note:** Usually LoadEntityTemplate is preferred over this.

### DeleteEntity

Delete an entity.

**Fields:**
- `entity` (Entity): entity to delete

**Example:**
```json
{
  "DeleteEntity": {
    "entity": 42
  }
}
```

### DeleteClientEntity

Delete an entity on the client side only.

**Fields:**
- `entity` (Entity): entity to delete on client

**Example:**
```json
{
  "DeleteClientEntity": {
    "entity": 42
  }
}
```

### SpawnEntityRequestEvent

Request spawning an entity.

**Fields:**
- `entity_template` (string): template to spawn
- `params` (object): spawn parameters

**Example:**
```json
{
  "SpawnEntityRequestEvent": {
    "entity_template": "powerup_health",
    "params": {
      "position": {"x": 0.5, "y": 0.5}
    }
  }
}
```

### KillEntityRequestEvent

Request killing an entity.

**Fields:**
- `target` (Entity): entity to kill
- `reason` (string, optional): reason for killing

**Example:**
```json
{
  "KillEntityRequestEvent": {
    "target": 42,
    "reason": "out_of_bounds"
  }
}
```

### ModifyComponentRequestEvent

Modify a component on an entity.

**Fields:**
- `target` (Entity): entity to modify
- `component_name` (string): name of component to modify
- `modifications` (object): changes to apply

**Example:**
```json
{
  "ModifyComponentRequestEvent": {
    "target": 0,
    "component_name": "life:Health",
    "modifications": {
      "current": 100.0
    }
  }
}
```

---

## Inventory Events

### PickUp

Pick up an item entity.

**Fields:**
- `to_pick` (Entity): item entity to pick up
- `picker` (Entity): entity picking up the item

**Example:**
```json
{
  "PickUp": {
    "to_pick": 5,
    "picker": 0
  }
}
```

### UseItem

Use an item from inventory.

**Fields:**
- `consumer` (Entity): entity using the item
- `slot_item` (int): inventory slot index
- `nb_to_use` (int): quantity to use

**Example:**
```json
{
  "UseItem": {
    "consumer": 0,
    "slot_item": 0,
    "nb_to_use": 1
  }
}
```

### DropItem

Drop an item from inventory.

**Fields:**
- `consumer` (Entity): entity dropping the item
- `slot_item` (int): inventory slot index
- `nb_to_use` (int): quantity to drop

**Example:**
```json
{
  "DropItem": {
    "consumer": 0,
    "slot_item": 1,
    "nb_to_use": 1
  }
}
```

### RemoveItem

Remove an item from inventory without dropping.

**Fields:**
- `consumer` (Entity): entity to remove item from
- `slot_item` (int): inventory slot index
- `nb_to_use` (int): quantity to remove

**Example:**
```json
{
  "RemoveItem": {
    "consumer": 0,
    "slot_item": 2,
    "nb_to_use": 1
  }
}
```

---

## Interaction Events

### InteractionZoneEvent

Triggered when entities are within an interaction zone.

**Fields:**
- `source` (Entity): entity with interaction zone
- `radius` (double): interaction radius
- `candidates` (array of Entity): entities within the zone

**Note:** This event is emitted by the interaction system.

### EnteredZone

Triggered when an entity enters a zone.

**Fields:**
- `zone` (Entity): zone entity entered
- `player` (Entity): entity that entered

**Note:** This event is emitted by the border interaction system.

### LeftZone

Triggered when an entity leaves a zone.

**Fields:**
- `zone` (Entity): zone entity left
- `player` (Entity): entity that left

**Note:** This event is emitted by the border interaction system.

---

## Network Events

### StateTransfer

Transfer game state to a client.

**Fields:**
- `client_id` (int): client to send state to

**Note:** Internal network event.

### PlayerReady

Indicate a player is ready.

**Fields:**
- `client_id` (int): client ID that is ready

**Example:**
```json
{
  "PlayerReady": {
    "client_id": 1
  }
}
```

### WantReady

Request ready state.

**Example:**
```json
{
  "WantReady": {}
}
```

### Disconnection

Player disconnected from server.

**Note:** This event is emitted by the network system.

### ResetClient

Reset client game state.

**Fields:**
- `sequence` (int): sequence number for synchronization

**Note:** Internal network event.

---

## System Events

### ShutdownEvent

Shutdown the application.

**Fields:**
- `reason` (string): reason for shutdown
- `exit_code` (int): exit code (default: 0)

**Example:**
```json
{
  "ShutdownEvent": {
    "reason": "user_quit",
    "exit_code": 0
  }
}
```

### CleanupEvent

Trigger cleanup operations.

**Fields:**
- `trigger` (string): what triggered the cleanup

**Example:**
```json
{
  "CleanupEvent": {
    "trigger": "scene_change"
  }
}
```

### LoadPluginEvent

Dynamically load a plugin.

**Fields:**
- `path` (string): path to plugin file
- `params` (object, optional): plugin parameters

**Example:**
```json
{
  "LoadPluginEvent": {
    "path": "plugins/custom_plugin.so",
    "params": {}
  }
}
```

### LoadConfigEvent

Load configuration from file.

**Fields:**
- `path` (string): path to config file

**Example:**
```json
{
  "LoadConfigEvent": {
    "path": "config/settings.json"
  }
}
```

### LogEvent

Log a message.

**Fields:**
- `name` (string): logger name
- `level` (string): "DEBUG", "INFO", "WARNING", "ERROR", "CRITICAL"
- `message` (string): log message

**Example:**
```json
{
  "LogEvent": {
    "name": "game",
    "level": "INFO",
    "message": "Player spawned"
  }
}
```

---

## Using Events in JSON

Events can be triggered from various places in JSON configuration:

### In Item On-Use Actions

```json
"inventory:Inventory": {
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
      }
    }
  ]
}
```

### In Action Triggers

```json
"actions:ActionTrigger": {
  "trigger_type": "on_create",
  "actions": [
    {
      "PlaySoundEvent": {
        "entity": "@self",
        "name": "spawn",
        "volume": 80.0,
        "pitch": 1.0,
        "loop": false
      }
    }
  ]
}
```

### Entity References

- `@self` - Current entity
- `0`, `1`, etc. - Direct entity ID
- Hooks can be used where entity IDs are expected

---

## Event Handler Priority

When subscribing to events in code, handlers can specify priority:
- Higher priority = runs first
- Default priority = 0
- Range: typically -100 to 100

**Example in C++:**
```cpp
event_manager.on<DamageEvent>("damage_handler", 
    [](const DamageEvent& evt) {
        // Handle damage
        return false;  // or PREVENT_DEFAULT to stop other handlers
    },
    10  // Higher priority
);
```

---

## Event Propagation

Events can be stopped from propagating to other handlers by returning `PREVENT_DEFAULT` from an event handler. This is useful for implementing event cancellation logic.
