#pragma once

namespace game {
enum class TileType { Plains, Mountain };
enum class TileContent { None, Forest, Factory, OpenPitMine };

// A plains tile has at most one item. Mountains should have no content.
// A tile owns its values; no separately allocated item is needed.
struct Tile {
    TileType type = TileType::Plains;
    TileContent content = TileContent::None;
    bool alive = true; // Selects live/dead factory and mine sprites.
    float trees = 0.0f;
    float pollutionGainRate = 0.0f; // Actual pollution change in the last resource update.
    float oreMineRate = 0.0f; // Ground ore removed per resource update, including waste.
    float oreWasteRate = 0.0f; // Portion of the removal that is not collected.
    float treesDecayRate = 0.0f; // Per resource update: positive growth, negative decay.
    float throughput = 0.0f; // Factory ore consumption per resource update; zero when inactive.
    float efficiency = 0.0f; // Factory capital produced per ore consumed.
    float pollution = 0.0f;
    float ore = 0.0f; // Unmined reserve; initialized after the terrain layout.
    float initialOre = 0.0f; // Original reserve, used for cumulative mining pollution.
};
}
