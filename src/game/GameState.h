#pragma once

#include "game/Tile.h"
#include "app/Config.h"
#include <array>

namespace game {
struct InputState;
inline constexpr int boardColumns = config::boardColumns;
inline constexpr int boardRows = config::boardRows;
enum class Scene { Title, Playing, End };

struct GameState {
    Scene scene = Scene::Title;
    // Row-major: index = row * boardColumns + column.
    std::array<Tile, boardColumns * boardRows> tiles{};
    float ore = 0.0f; // Collected ore, separate from each tile's unmined reserve.
    float liquidCapital = 0.0f;
    float buildingCapital = 0.0f; // Combined value of all buildings.
    int selectedTile = -1; // Row-major index; -1 means normal board view.
    float insufficientResourcesRemaining = 0.0f;
    int updatesSinceResources = 0; // Counts unpaused gameplay updates.
    float oreGainPerUpdate = 0.0f; // Last resource tick's net gain, including factory consumption.
    float capitalGainPerUpdate = 0.0f; // Last resource tick's produced capital.
    bool leftTutorialVisible = true;
    bool rightTutorialVisible = true;
};

GameState makeInitialGameState();
// Mutable reference: this function is allowed to change the game.
void updateGame(GameState& state, const InputState& input, float deltaSeconds);
}
