#pragma once

#include "game/Tile.h"
#include <array>

namespace game {
struct InputState;
inline constexpr int boardColumns = 5;
inline constexpr int boardRows = 5;
enum class Scene { Title, Playing, End };

struct GameState {
    Scene scene = Scene::Title;
    // Row-major: index = row * boardColumns + column.
    std::array<Tile, boardColumns * boardRows> tiles{};
    float liquidCapital = 0.0f;
    float buildingCapital = 0.0f;
};

GameState makeInitialGameState();
// Mutable reference: this function is allowed to change the game.
void updateGame(GameState& state, const InputState& input, float deltaSeconds);
}
