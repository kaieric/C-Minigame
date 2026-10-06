#include "render/Renderer.h"
#include "app/Config.h"
#include "game/GameState.h"
#include "raylib.h"

namespace game {
namespace {
void drawCenteredText(const char* text, int y, int fontSize)
{
    DrawText(text, (config::windowWidth - MeasureText(text, fontSize)) / 2,
             y, fontSize, RAYWHITE);
}

Color tileColor(TileType type)
{
    switch (type) {
    case TileType::Plains: return Color{150, 162, 112, 255};
    case TileType::Forest: return Color{54, 104, 66, 255};
    case TileType::Mountain: return Color{100, 105, 112, 255};
    }
    return MAGENTA; // Makes an unexpected value visible during development.
}
}

void drawGame(const GameState& state)
{
    BeginDrawing();
    ClearBackground(BLACK);
    if (state.scene == Scene::Title || state.scene == Scene::End) {
        drawCenteredText("Click to begin", config::windowHeight / 2 - 12, 24);
    } else {
        const int left = (config::windowWidth - boardColumns * config::tileSize) / 2;
        const int top = (config::windowHeight - boardRows * config::tileSize) / 2;
        for (int row = 0; row < boardRows; ++row) {
            for (int column = 0; column < boardColumns; ++column) {
                const Tile& tile = state.tiles[row * boardColumns + column];
                const int x = left + column * config::tileSize;
                const int y = top + row * config::tileSize;
                DrawRectangle(x, y, config::tileSize, config::tileSize, tileColor(tile.type));
                DrawRectangleLines(x, y, config::tileSize, config::tileSize, BLACK);
            }
        }
    }
    EndDrawing();
}
}
