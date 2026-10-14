#pragma once

#include "raylib.h"

namespace game {
struct GameState;
struct Tile;
struct InputState;
// Owns GPU textures. Construct after InitWindow; destroy before CloseWindow.
class Renderer {
public:
    Renderer();
    ~Renderer();
    // Copying texture handles would cause two owners to unload the same texture.
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;
    void advanceAnimation(float deltaSeconds, int selectedTile = -1);
    void drawGame(const GameState& state, const InputState& input) const;

private:
    struct TerrainAnimation {
        Texture2D texture{}; // All GIF frames stacked vertically in one texture.
        int frameCount = 0;
        int frameHeight = 0;
    };
    static TerrainAnimation loadTerrain(const char* filename);
    void drawSprite(const TerrainAnimation& sprite, int x, int y, int size,
                    double seconds, bool pingPong = false) const;
    void drawTile(const Tile& tile, int x, int y, int size, double seconds) const;
    void drawSelection(const Tile& tile, int centerX, int centerY) const;
    TerrainAnimation plains_{};
    TerrainAnimation forest_{};
    TerrainAnimation mountain_{};
    TerrainAnimation liveFactory_{};
    TerrainAnimation deadFactory_{};
    TerrainAnimation liveOpenPitMine_{};
    TerrainAnimation deadOpenPitMine_{};
    double animationSeconds_ = 0.0;
    double selectionAnimationSeconds_ = 0.0;
    int animatedSelection_ = -1;
};
}
