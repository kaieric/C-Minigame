#include "game/GameState.h"
#include "app/Config.h"
#include "input/Input.h"
#include "game/ResourceUpdate.h"

#include <algorithm>

namespace game {
namespace {
void checkWin(GameState& state)
{
    if (state.liquidCapital + state.buildingCapital >= config::winningTotalCapital) {
        state.scene = Scene::End;
        state.selectedTile = -1;
    }
}
}

GameState makeInitialGameState()
{
    using T = TileType;
    using C = TileContent;
    // One row per line, left to right. The map is identical on each restart.
    // Tile{terrain, content, alive}; omitted fields use their default values.
    constexpr Tile p{T::Plains};
    constexpr Tile f{T::Plains, C::Forest};
    constexpr Tile m{T::Mountain};
    constexpr Tile factory{T::Plains, C::Factory};
    constexpr Tile mine{T::Plains, C::OpenPitMine};
    constexpr std::array<Tile, boardColumns * boardRows> tiles = {
        m,       m,    f, m, f, f, p, p, p, p,
        m,       f,    m, f, f, p, p, p, p, p,
        f,       m,    f, f, p, p, p, p, f, p,
        m,       f,    f, p, p, p, p, f, p, p,
        f,       f,    p, p, p, p, f, p, f, f,
        p,       p,    p, p, p, p, p, f, f, m,
        p,       p,    p, p, p, p, f, f, m, f,
        p,       p,    p, p, p, f, f, m, f, m,
        mine,    p,    p, p, p, p, f, f, m, m,
        factory, mine, p, p, p, f, m, f, m, m,
    };
    GameState state;
    state.tiles = tiles;
    state.ore = config::startingOre;
    state.liquidCapital = config::startingLiquidCapital;

    // Second pass: every neighbor's terrain is now available.
    const auto isMountain = [&state](int row, int column) {
        return row >= 0 && row < boardRows && column >= 0 && column < boardColumns
            && state.tiles[row * boardColumns + column].type == TileType::Mountain;
    };
    for (int row = 0; row < boardRows; ++row) {
        for (int column = 0; column < boardColumns; ++column) {
            Tile& tile = state.tiles[row * boardColumns + column];
            if (tile.type != TileType::Plains) continue;
            // Only edge-sharing neighbors count, not diagonals.
            const int adjacentMountains = isMountain(row - 1, column)
                + isMountain(row + 1, column)
                + isMountain(row, column - 1)
                + isMountain(row, column + 1);
            tile.ore = config::basePlainsOre
                + adjacentMountains * config::orePerAdjacentMountain;
            tile.initialOre = tile.ore;
            switch (tile.content) {
            case TileContent::Factory:
                state.buildingCapital += config::factoryCapitalCost;
                break;
            case TileContent::OpenPitMine:
                state.buildingCapital += config::openPitMineCapitalCost;
                break;
            case TileContent::None:
                break;
            case TileContent::Forest:
                tile.trees = config::startingForestTrees;
                break;
            }
            updateTileResourceRates(tile);
        }
    }
    return state;
}

void updateGame(GameState& state, const InputState& input, float deltaSeconds)
{
    if (input.primaryPressed && state.scene != Scene::Playing) {
        state = makeInitialGameState();
        state.scene = Scene::Playing;
        return;
    }
    if (state.scene != Scene::Playing) {
        return;
    }
    checkWin(state);
    if (state.scene == Scene::End) return;

    state.insufficientResourcesRemaining = std::max(
        0.0f, state.insufficientResourcesRemaining - deltaSeconds);

    if (input.dismissLeftTutorial && state.leftTutorialVisible) {
        state.leftTutorialVisible = false;
        return; // Consume the UI click without selecting a tile.
    }
    if (input.dismissRightTutorial && state.rightTutorialVisible) {
        state.rightTutorialVisible = false;
        return;
    }

    if (state.selectedTile >= 0) {
        Tile& tile = state.tiles[state.selectedTile];
        if (input.anyMousePressed || input.primaryPressed || input.anyKeyPressed) {
            // A mouse click always dismisses. Construction is allowed only on
            // an empty plains tile, and either success or failure closes it.
            if (!input.anyMousePressed && !input.primaryPressed
                && tile.type == TileType::Plains && tile.content == TileContent::None
                && (input.factoryPressed || input.minePressed)) {
                const bool factory = input.factoryPressed;
                const float capitalCost = factory ? config::factoryCapitalCost
                                                  : config::openPitMineCapitalCost;
                const float oreCost = factory ? config::factoryOreCost
                                              : config::openPitMineOreCost;
                if (state.liquidCapital >= capitalCost && state.ore >= oreCost) {
                    state.liquidCapital -= capitalCost;
                    state.ore -= oreCost;
                    state.buildingCapital += capitalCost;
                    tile.content = factory ? TileContent::Factory : TileContent::OpenPitMine;
                    tile.alive = true;
                    updateTileResourceRates(tile);
                } else {
                    state.insufficientResourcesRemaining = config::insufficientResourcesSeconds;
                }
            }
            state.selectedTile = -1;
        }
        return; // Board simulation stays paused, including the dismissal frame.
    }

    if (input.primaryPressed) {
        const int left = (config::windowWidth - boardColumns * config::tileSize) / 2;
        const int top = (config::windowHeight - boardRows * config::tileSize) / 2;
        const float x = input.mouseX - left;
        const float y = input.mouseY - top;
        if (x >= 0 && x < boardColumns * config::tileSize
            && y >= 0 && y < boardRows * config::tileSize) {
            const int column = static_cast<int>(x / config::tileSize);
            const int row = static_cast<int>(y / config::tileSize);
            state.selectedTile = row * boardColumns + column;
            return;
        }
    }

    if (++state.updatesSinceResources >= config::resourceUpdateInterval) {
        updateResources(state);
        state.updatesSinceResources = 0;
        checkWin(state);
    }
}
}
