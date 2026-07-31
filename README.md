# SDL Platformer

A 2D side-scrolling platformer built in C++20 with [SDL3](https://github.com/libsdl-org/SDL). The player runs, jumps, and shoots their way through a level populated by shambling enemies, with parallax-scrolling backgrounds and a simple tile-based level.

This is a personal/learning project, still under active development — some systems (like Tiled map loading) are partially wired up but not yet used by the game.

## Features

- Player movement: run, jump, shoot, and a directional slide when reversing direction while running
- Simple enemy AI that chases the player when nearby and can be damaged/killed by bullets
- Axis-aligned bounding box (AABB) collision detection and resolution between the player, enemies, bullets, and level geometry
- Sprite-sheet based animation system (idle, run, shoot, slide, hit/death states, etc.)
- 4-layer parallax-scrolling background
- Sound effects and looping level music via SDL3_mixer
- Debug overlay (state, bullet count, grounded flag) toggled with `F5`

## Tech stack

- **Language:** C++20
- **Build system:** CMake (3.8+)
- **Libraries:**
  - [SDL3](https://github.com/libsdl-org/SDL) — windowing, rendering, input
  - [SDL3_image](https://github.com/libsdl-org/SDL_image) — texture loading (PNG sprite sheets)
  - [SDL3_mixer](https://github.com/libsdl-org/SDL_mixer) — audio playback/mixing
  - [GLM](https://github.com/g-truc/glm) — vector math (`glm::vec2` for position/velocity/acceleration)
  - [tinyxml2](https://github.com/leethomason/tinyxml2) — bundled directly in the source tree, used by the (in-progress) Tiled map loader

These libraries are resolved via CMake's `find_package`, so they need to be discoverable on your system (e.g. through a vcpkg toolchain file or a system install) — there's no vendored/manifest dependency setup in this repo.

## Building

The provided `CMakePresets.json` targets Windows + MSVC (`cl.exe`) with the Ninja generator, in Debug/Release and x86/x64 configurations. The general flow:

1. Make sure SDL3, SDL3_image, SDL3_mixer, and GLM are installed and discoverable by CMake (e.g. via vcpkg with `CMAKE_TOOLCHAIN_FILE` set, either globally or in a local `CMakeUserPresets.json`).
2. Configure and build using one of the presets, e.g.:
   ```
   cmake --preset x64-debug
   cmake --build --preset x64-debug
   ```
3. Run the resulting `SDLPlatformer` executable **from the repository root** — asset paths (`assets/...`) are relative to the working directory, not the executable location.

The top-level `CMakeLists.txt` just includes the `SDLPlatformer/` subdirectory, which defines the actual executable target and links the libraries above.

## Controls

| Key | Action |
|---|---|
| `A` / `Left Arrow` | Move left |
| `D` / `Right Arrow` | Move right |
| `Space` | Jump |
| `J` | Shoot |
| `F5` | Toggle debug overlay |
| `Escape` | Quit |

## Project structure

```
SDLPlatformer/
  SDLPlatformer.cpp        Entry point, game loop, physics/collision, rendering
  tmx.cpp / include/tmx.h  Tiled (.tmx) map loader (work in progress, not yet used by the game loop)
  tinyxml2.cpp/.h          Bundled XML parser used by the TMX loader
  include/
    SDLPlatformer.h        Global constants (tile size, map dimensions, layer indices)
    sdlstate.h              SDLState: window/renderer handles, screen size, keyboard state
    gamestate.h              GameState: all live game objects, camera viewport, scroll state
    gameobject.h            GameObject definition, ObjectType/*State enums, per-type data union
    resources.h              Resources: texture/audio loading and animation setup
    animation.h              Animation: frame-based sprite animation driven by a Timer
    timer.h                   Timer: simple countdown/elapsed-time helper
assets/
  idle.png, run.png, shoot.png, slide.png, ...   Player sprite sheets
  enemy*.png                                      Enemy sprite sheets
  bullet*.png                                     Bullet sprite sheets
  bg/                                              Parallax background layers
  tiles/                                           Level tile textures
  audio/                                           Sound effects and music
  maps/                                            Tiled (.tmx/.tsx) map files (not yet loaded at runtime)
```

## Architecture overview

### Game loop (`main`, in `SDLPlatformer.cpp`)

Each frame:
1. Compute `deltaTime` from `SDL_GetTicks()`.
2. Poll SDL events — quit, window resize, and keyboard input (routed to `handleKeyInput`).
3. Call `update()` on every `GameObject` (level layers, then bullets).
4. Recompute the camera (`gs.mapViewport`) to follow the player horizontally.
5. Clear the screen and draw, back to front: static background image → parallax layers → background tiles → all game object layers → bullets → foreground tiles → optional debug overlay.
6. Present the frame.

### `GameObject` (`include/gameobject.h`)

A single struct represents every entity in the game — the player, enemies, bullets, and level/tile geometry — distinguished by an `ObjectType` enum (`player`, `level`, `enemy`, `bullet`). Type-specific state (e.g. `PlayerState`, `EnemyState`, `BulletState`, hit points, timers) lives in an `ObjectData` **union**, so only one variant's data is meaningful at a time based on `type`.

Common fields include position/velocity/acceleration (as `glm::vec2`), a collider rectangle, the current texture and animation, and flags like `dynamic` (affected by physics) and `grounded`.

### `GameState` (`include/gamestate.h`)

Owns all live objects for the current level:
- `layers[LAYER_IDX_LEVEL]` / `layers[LAYER_IDX_CHARACTERS]` — the two object layers that participate in collision and per-object `update()`
- `backgroundTiles` / `foregroundTiles` — purely visual tiles drawn behind/in front of everything else
- `bullets` — a pool of bullet objects (inactive bullets are recycled instead of being reallocated)
- `mapViewport` — the current camera rectangle, plus scroll offsets for the parallax layers
- `playerIndex` — index into `layers[LAYER_IDX_CHARACTERS]` for the player; `player()` is a convenience accessor

### `Resources` (`include/resources.h`)

Loads and owns all textures, audio chunks, and animation definitions up front (`load()`), and frees textures on shutdown (`unload()`). Named `ANIM_*` constants map into per-entity `animations` vectors (e.g. `ANIM_PLAYER_IDLE` indexes into `playerAnims`).

### `Animation` / `Timer` (`include/animation.h`, `include/timer.h`)

`Timer` is a minimal countdown helper — `step(deltaTime)` advances it and reports when it wraps past its configured length, `reset()`/`isTimeout()` control and query state. `Animation` wraps a `Timer` to derive the current sprite-sheet frame index from elapsed time and a fixed frame count.

### Level construction (`createTiles`)

The current level is defined by three hardcoded 2D arrays (`map`, `foregroundMap`, `backgroundMap`) of small integer tile codes (ground, panel, enemy, player spawn, grass, brick). `createTiles` walks these arrays and instantiates the corresponding `GameObject`s into the appropriate `GameState` layer/collection. This is the actual source of the current level — `assets/maps/*.tmx` and the TMX loader below are not yet connected to it.

### Physics & collision (`update`, `checkCollision`, `collisionResponse`)

Each dynamic object gets gravity applied, has its animation advanced, moves per its type-specific state machine (player input/state transitions, enemy chase AI, bullet lifetime), and is then checked for AABB collisions against every object in the level layers via `SDL_GetRectIntersectionFloat`. `collisionResponse` resolves collisions differently depending on the pair of types involved — e.g. pushing the player out of solid geometry, knocking the player back off an enemy, or damaging/killing an enemy and switching a bullet to its "colliding" state on hit.

A separate 1px-tall sensor rectangle just below each object's collider is used to determine `grounded` state each frame.

### Rendering (`drawObject`, `drawParallaxBackground`)

Sprites are drawn by slicing the current animation frame (or a fixed `spriteFrame`) out of a sprite-sheet texture and blitting it at the object's world position minus the camera's viewport offset, with horizontal flipping based on facing direction. Damaged enemies get a temporary reddish tint (`shouldFlash`) via `SDL_GetTextureColorModFloat`. Background layers scroll at different rates relative to player velocity to produce a parallax effect.

## Known limitations / in-progress work

- **Tiled map loading parses correctly but is still unused.** `tmx::loadMap` (`tmx.cpp`) parses `.tmx`/`.tsx` XML into `tmx::Map`/`TileSet`/`Layer` structures (its earlier attribute-name and tileset-parsing bugs have been fixed), but nothing in the game loop calls it yet — `createTiles`'s hardcoded arrays are still the only way levels are built.
- The debug overlay and inline comments in `update()` reference a past bug with jumping/Y-velocity that may still be worth revisiting.
- There's no automated test suite; verifying changes currently means building and playing the game.
