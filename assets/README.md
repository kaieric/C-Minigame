# Assets

Put tile images here when you are ready to replace the colored rectangles.
No assets are loaded yet. The initial game uses raylib's built-in font.

Load textures after `InitWindow`, retain them across frames, and unload them
before `CloseWindow`. A future resource owner can handle this using RAII.
When adding assets, also add copying to the desktop build and Emscripten
`--preload-file` packaging to the web build. Avoid depending on an IDE's working directory.
