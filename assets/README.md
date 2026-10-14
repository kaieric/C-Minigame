# Assets

Terrain mapping: Plains uses `Plains.gif` and Mountain uses `Mountain.gif`.
Plains contents render over the terrain: Forest uses `PinkTree.gif`; Factory
uses `LiveFactory.gif` or `DeadFactory.gif`; OpenPitMine uses
`LiveOpenPitMine.gif` or `DeadOpenPitMine.gif`, selected by `Tile::alive`.
The mine sprites have transparent pixels, exposing the plains underneath.
The current forest and factory GIFs are opaque and cover the plains sprite.
`Tree.gif` is unused. All GIF frames are decoded into texture atlases,
with point filtering for crisp pixel art. Plains, mountains, factories, and mines loop at 10 fps;
PinkTree eases quadratically from frame 1 to 13 and back in 2.4 seconds. The font is raylib's built-in font.

`Renderer` owns textures using RAII: it loads them after `InitWindow` and
unloads them before `CloseWindow`. A failed load leaves a colored fallback tile.
CMake copies this folder beside the desktop executable on every build.
Desktop paths use the executable's directory, independent of the working directory.
The web build preloads this folder into Emscripten's virtual filesystem.
