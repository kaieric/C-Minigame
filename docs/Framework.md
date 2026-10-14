# Your game framework

This is a starting point for you to build on. It opens a 1200×600 window,
shows “Click to begin,” and displays a deterministic 10×10 board of mixed terrain
when clicked. Closing the window exits on desktop; Escape dismisses a tile selection.
Terrain uses all 13 frames of each 16×16 GIF, scaled to 64×64 with point filtering.
Resource simulation and construction are implemented; there are no scanlines, sound, or map editor yet.
Reaching `winningTotalCapital` (500 by default), including fixed building capital,
shows a win screen. Click that screen to restart with the initial board and resources.

During play, the bottom row shows collected Ore, Liquid Capital, and Total
Capital (liquid plus building capital), using the title font in `#c5c4cc`.
Each counter includes a signed bracketed change from the last resource tick;
ore shows mining output minus factory consumption, and both capital counters
show produced capital. Construction spending is excluded from these rates.
Rates start at zero until the first resource tick.
Collected ore starts at `startingOre` and is separate from unmined `Tile::ore`.
After the map is initialized, each plains tile receives `basePlainsOre` plus
`orePerAdjacentMountain` for each edge-sharing mountain (not diagonals).
Both amounts default to 90 and can be changed in `src/app/Config.h`.
Keep shared tuning values in that file. Factory capital value defaults to 10
and mine capital value to 3. Starting buildings, including dead ones, add those values
to `buildingCapital`; construction adds the building's capital cost to this total.

The initial map has one factory and two mines on plains in the bottom-left
clearing. Mountains and forests are concentrated toward the top-left and
bottom-right. Edit the row-by-row array in `makeInitialGameState()` to change it.
Selected empty plains show ground ore and pollution alongside construction
options; mountains show pollution. All selection pollution text is red.

During normal play, hold P for pollution intensity, O for remaining ore, or I
for the sign of the last resource tick's pollution change. O is gray at full
original ore reserves and transparent at zero. I is green for any positive
change, red for any negative change, and transparent at zero, with no scaling.
All three overlays vanish on key release and are hidden in selection and menus.
If keys overlap, I takes priority over O, then P. Overlay colors and maximum
opacity (80 out of 255 by default) are adjustable in `Config.h`.

Edit `leftTutorialText` and `rightTutorialText` in `Config.h` to change the side
instructions. Text wraps within the configured width; `\n` adds an explicit line
break. Set a text to `""` to hide its tutorial. Position, height, font size, and
spacing are configurable. Clicking `dismiss` hides that side until a new game.

## Resource simulation

`src/game/ResourceUpdate.cpp` performs one resource tick after every
`resourceUpdateInterval` unpaused game updates (180 by default, about three
seconds at 60 FPS). Selecting a tile pauses the count; closing the selection
resumes it without catching up paused updates. All balancing constants are in
the resource interactions section of `src/app/Config.h`.

Each tick forecasts rates from starting pollution. Forests start with 120 trees,
grow by `4 - pollution / 10`, and stay between zero and 120 trees; reaching zero
removes the forest. Mines collect `3 + 0.6 * pollution / 10` ore and remove an
additional `0.2 * pollution / 10` as waste. A partial final extraction reduces
collection and waste proportionally. Exhausted mines remain as dead buildings.

Factories process `10 + 5 * pollution / 10` ore, at an efficiency of
`0.05 * (1 - 0.03 * pollution / 10)` capital per ore. Factories are considered
from highest potential capital output to lowest; equal outputs use board order.
Each activation must leave enough ore for one mine after forecast usable mining
output and previously activated factories. Inactive factories use the dead
sprite and are reconsidered on the next tick. Their displayed throughput is zero.

Pollution runs last. An operating mine adds 0.005 pollution per ore cumulatively
removed from its original reserve, including waste and the current extraction.
It emits on its final extraction tick, then stops. An active factory adds 0.2
pollution per ore processed. Every tile also gains 0.005 per total pollution and
loses 0.005 per remaining forest tree in its 3x3 neighborhood, including itself.
Neighborhoods use post-production trees and a shared snapshot of pre-pollution
values. Pollution is bounded between zero and 100. Displayed resource rates come
from the latest resource tick; pollution gain shows the actual bounded change.

The regression tests cover the default tuning values and can be run after building:

```sh
ctest --test-dir build --output-on-failure
```

## Build and run

You need CMake 3.24 or newer and a C++ compiler. On macOS, the Xcode
Command Line Tools provide the compiler. From the workspace root:

```sh
cmake -S . -B build
cmake --build build
./build/finite_yield
```

The first configure downloads raylib 5.5 from its official repository,
so it needs internet access. CMake builds raylib for you; you do not need
to install it separately. Build output stays in the ignored `build/` directory.
Run the build command again after changing code. Adding a `.cpp` file also
requires listing it in `CMakeLists.txt`.

## File map

| File or folder | Responsibility |
| --- | --- |
| `src/main.cpp` | Entry point; calls `game::run()` |
| `src/app/GameLoop.cpp` | Window lifetime and frame scheduling |
| `src/app/Config.h` | Window dimensions, frame target, tile display size |
| `src/input/` | Reads raylib input into an `InputState` snapshot |
| `src/game/Tile.h` | Terrain, contents, and per-tile values |
| `src/game/GameState.h` | Board, scene, and capital owned by the game |
| `src/game/GameState.cpp` | Initial board and gameplay updates |
| `src/game/ResourceUpdate.cpp` | Resource forecasting, production, and pollution |
| `src/render/` | Owns terrain textures and draws the current state with raylib |
| `assets/` | Future images and asset-lifetime notes |
| `CMakeLists.txt` | Dependency and executable build instructions |
| `README.md` | Your original game design |

Headers (`.h`) declare the types and functions other files can use.
Source files (`.cpp`) define what those functions do. `#pragma once` prevents
a header from being included repeatedly in one compilation unit.
The `game` namespace groups our names and avoids collisions with libraries.

## What happens each frame?

```text
main → run → initialize window and game state
             ↓
             readInput → updateGame → drawGame
             ↑                         ↓
             └──────── next frame ──────┘
             ↓ when desktop window closes
             CloseWindow
```

`readInput()` converts mouse position and a click into ordinary values.
`updateGame()` receives those values and changes the simulation.
`drawGame()` reads the result and draws it. Keep harvesting, pollution,
and purchases out of the renderer: drawing should never advance the economy.

`deltaSeconds` means elapsed seconds for the frame. A rate of two wood per
second should add `2.0f * deltaSeconds`, rather than two wood each frame.
The loop caps this time at 0.1 seconds to avoid large jumps after interruptions.
That intentionally discards elapsed time during long stalls; this skeleton
does not implement catch-up simulation or a fixed timestep.

## State and ownership in C++

`GameState` owns an `std::array` of 100 tiles directly. No `new`, `delete`,
inheritance, or heap-owned tile objects are needed. `Tile` owns its terrain,
contents, live/dead flag, and numeric values. Terrain is plains or mountain.
The content enum allows one forest, factory, or open pit mine per plains tile,
or `None` for an empty tile. Mountains should have `None` as their content.
Rendering draws terrain first and then the contents of plains.

The board is row-major: `(column, row)` maps to
`row * boardColumns + column`. Coordinates begin at zero. Check bounds
before using a calculated index; `std::array::operator[]` does not check them.

`GameState&` is a borrowed reference to the original state: updates change
that original. `const GameState&` is a borrowed, read-only reference: the
renderer cannot change it. Neither reference owns or copies the state.
`makeInitialGameState()` returns a value, and assigning that value resets
the board, scene, and capital together.

The desktop loop owns state as a local variable. The browser loop uses a
static variable because callbacks must have valid state after registration.
The `void*` callback parameter is required by Emscripten's C interface;
it points to a `FrameContext` containing state and the renderer, and is converted back inside `frame()`.
Do not use that pattern for ordinary gameplay functions.

`Renderer` loads textures once after the window opens and unloads them in its
destructor. On desktop, the inner scope in `run()` ends before `CloseWindow`,
so cleanup happens while the graphics context still exists. This is RAII:
resource cleanup follows the lifetime of its owner. Copying the renderer is
disabled so two objects cannot accidentally unload the same texture.

## Suggested implementation order

1. Edit the fixed tile array in `makeInitialGameState()` to change the map.
   Use `Tile{TileType::Plains, TileContent::Factory}` for a live factory, or
   add `false` as the third value for a dead factory. The same form works for
   `OpenPitMine`; use `Forest` for trees. Omitted fields keep their defaults.
   Initialize forest wood there when implementing forest rules.
2. Convert a click to a board coordinate. The renderer currently centers
   the board; share that layout calculation with input handling when needed.
   Reject clicks outside the board before indexing it.
3. Add building placement in the game update: validate plains, an empty
   content slot, and available capital before spending anything.
4. Add forest regeneration and sawmill harvesting using rates per second.
5. Add factory production and define adjacency, costs, and production rates.
6. Add pollution diffusion. Compute changes from a snapshot of the old
   pollution, then apply them together so traversal order does not bias spread.
7. Check `liquidCapital + buildingCapital` against your chosen victory target
   and switch to `Scene::End`.
8. Add scanlines and any desired animation after the rules are understandable.

Resource production and construction from these steps are implemented. Starting
liquid capital is controlled by `startingLiquidCapital`. Keep
functions small; split new simulation functions into more files only when
their responsibilities become substantial. You do not need a class for every system.

## Browser build

Install and activate the Emscripten SDK first. In a shell with `emcmake`
available, use a separate build directory:

```sh
emcmake cmake -S . -B build-web -DPLATFORM=Web
cmake --build build-web
python3 -m http.server 8000 --directory build-web
```

Open `http://localhost:8000/finite_yield.html`. Serve the generated HTML,
JavaScript, and WebAssembly files together. A browser owns frame scheduling,
so this build uses an Emscripten callback instead of a blocking desktop loop.
The web path is included in the framework; it still needs testing with your SDK.

## References

- [raylib function cheatsheet](https://www.raylib.com/cheatsheet/cheatsheet.html)
- [Official raylib CMake template](https://github.com/raysan5/raylib-game-template/blob/main/CMakeLists.txt)
- [Official browser loop example](https://github.com/raysan5/raylib/blob/master/examples/core/core_basic_window_web.c)

## Terrain animation

The renderer decodes each GIF once with `LoadImageAnim`, uploads its frames
as a vertical texture atlas, and frees the CPU image. Rendering selects a
16×16 rectangle from the atlas; it does not reload GIFs each frame.

Plains and mountains show one frame every 0.1 seconds, wrapping after frame
13 (a 1.3-second cycle). PinkTree goes 1 → 13 → 1 in 2.4 seconds: each leg
contains 12 transitions over 1.2 seconds, averaging 10 transitions per second.
All tiles of the same type share the same animation phase.

`src/render/Animation.h` maps elapsed seconds to zero-based frame indices.
For PinkTree, normalized progress `u` uses `2*u*u` in the first half and
`1 - 2*(1-u)*(1-u)` in the second half. This piecewise quadratic curve has
continuous velocity: constant acceleration followed by constant deceleration,
with zero velocity at the ends. The return leg mirrors the outgoing leg.
Rounding to the nearest GIF frame keeps endpoint holds symmetric.
The artwork remains discrete; the timing creates the eased motion.

The visual clock advances only during play and uses uncapped elapsed time,
independent of the simulation cap. GIF-embedded frame delays are ignored.
