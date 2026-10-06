# Your game framework

This is a starting point for you to build on. It opens an 800×600 window,
shows “Click to begin,” and displays a deterministic 5×5 board of plains
when clicked. Closing the window or pressing Escape exits on desktop.
There are no economy rules, textures, scanlines, sound, or map editor yet.
The end scene exists but no victory condition triggers it yet.

## Build and run

You need CMake 3.24 or newer and a C++ compiler. On macOS, the Xcode
Command Line Tools provide the compiler. From the workspace root:

```sh
cmake -S . -B build
cmake --build build
./build/contamination
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
| `src/game/Tile.h` | Terrain, building, and per-tile values |
| `src/game/GameState.h` | Board, scene, and capital owned by the game |
| `src/game/GameState.cpp` | Initial board and gameplay updates |
| `src/render/` | Draws the current state with raylib |
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

`GameState` owns an `std::array` of 25 tiles directly. No `new`, `delete`,
inheritance, or heap-owned tile objects are needed. `Tile` owns its terrain,
building, and numeric values. The building enum allows at most one building
per tile while keeping terrain independent.

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
it points to this state and is converted back inside `frame()`.
Do not use that pattern for ordinary gameplay functions.

Only the window needs explicit setup and cleanup right now. When you add
textures, load them once after opening the window and unload them before
closing it. RAII means placing cleanup in an owning object's destructor,
so resources are released when the owner leaves scope. We can introduce
that owner when actual resources are added.

## Suggested implementation order

1. Edit `makeInitialGameState()` to create a small fixed mix of terrain.
   Initialize forest wood there. The renderer already colors all three types.
2. Convert a click to a board coordinate. The renderer currently centers
   the board; share that layout calculation with input handling when needed.
   Reject clicks outside the board before indexing it.
3. Add building placement in the game update: validate plains, an empty
   building slot, and available capital before spending anything.
4. Add forest regeneration and sawmill harvesting using rates per second.
5. Add factory production and define adjacency, costs, and production rates.
6. Add pollution diffusion. Compute changes from a snapshot of the old
   pollution, then apply them together so traversal order does not bias spread.
7. Check `liquidCapital + buildingCapital` against your chosen victory target
   and switch to `Scene::End`.
8. Add textures and scanlines after the rules are understandable.

These are suggested steps, not implemented mechanics. The starter capital
is zero; choose an initial amount when you implement purchasing. Keep
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

Open `http://localhost:8000/contamination.html`. Serve the generated HTML,
JavaScript, and WebAssembly files together. A browser owns frame scheduling,
so this build uses an Emscripten callback instead of a blocking desktop loop.
The web path is included in the framework; it still needs testing with your SDK.

## References

- [raylib function cheatsheet](https://www.raylib.com/cheatsheet/cheatsheet.html)
- [Official raylib CMake template](https://github.com/raysan5/raylib-game-template/blob/main/CMakeLists.txt)
- [Official browser loop example](https://github.com/raysan5/raylib/blob/master/examples/core/core_basic_window_web.c)
