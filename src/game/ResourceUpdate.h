#pragma once

namespace game {
struct GameState;
struct Tile;

// Initialize displayed rates from current pollution without producing resources.
void updateTileResourceRates(Tile& tile);
// One resource tick: forecast, allocate ore, produce resources, then pollution.
void updateResources(GameState& state);
}
