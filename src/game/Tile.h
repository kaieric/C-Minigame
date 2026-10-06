#pragma once

namespace game {
enum class TileType { Plains, Forest, Mountain };
enum class BuildingType { None, Sawmill, Factory };

// A tile owns its values. A building is separate from the terrain type.
// These are placeholders for your mechanics, not implemented rules.
struct Tile {
    TileType type = TileType::Plains;
    BuildingType building = BuildingType::None;
    float wood = 0.0f;
    float storedLumber = 0.0f;
    float pollution = 0.0f;
};
}
