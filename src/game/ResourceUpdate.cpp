#include "game/ResourceUpdate.h"
#include "app/Config.h"
#include "game/GameState.h"

#include <algorithm>
#include <array>

namespace game {
namespace {
static_assert(config::resourceUpdateInterval > 0);
static_assert(config::pollutionRateStep > 0.0f);
static_assert(config::pollutionInteractionRadius >= 0);
static_assert(config::forestTreeLimit >= 0.0f);
static_assert(config::minimumPollution <= config::maximumPollution);

struct ResourceRates {
    float treeChange = 0.0f;
    float mineYield = 0.0f;
    float mineWaste = 0.0f;
    float throughput = 0.0f;
    float efficiency = 0.0f;
};

ResourceRates calculateRates(const Tile& tile)
{
    ResourceRates rates;
    if (tile.type != TileType::Plains) return rates;
    const float pollutionSteps = std::clamp(tile.pollution,
        config::minimumPollution, config::maximumPollution) / config::pollutionRateStep;
    switch (tile.content) {
    case TileContent::None: break;
    case TileContent::Forest:
        rates.treeChange = config::forestBaseGrowth
            - pollutionSteps * config::forestGrowthLossPerPollutionStep;
        break;
    case TileContent::OpenPitMine:
        if (tile.alive && tile.ore > 0.0f) {
            rates.mineYield = std::max(0.0f, config::baseMineRate
                + pollutionSteps * config::acceleratedMineRate);
            rates.mineWaste = std::max(0.0f, pollutionSteps * config::oreWastagePerPollutionStep);
        }
        break;
    case TileContent::Factory:
        // Inactivity is temporary: every factory is reconsidered each tick.
        rates.throughput = std::max(0.0f, config::factoryThroughput
            + pollutionSteps * config::factoryThroughputGainPerPollutionStep);
        rates.efficiency = std::max(0.0f, config::factoryEfficiency
            * (1.0f - pollutionSteps * config::factoryEfficiencyLossPerPollutionStep));
        break;
    }
    return rates;
}

void storeRates(Tile& tile, const ResourceRates& rates)
{
    tile.treesDecayRate = rates.treeChange;
    tile.oreMineRate = tile.alive ? rates.mineYield + rates.mineWaste : 0.0f;
    tile.oreWasteRate = tile.alive ? rates.mineWaste : 0.0f;
    tile.throughput = tile.alive ? rates.throughput : 0.0f;
    tile.efficiency = rates.efficiency;
}

struct ResourcePlan {
    ResourceRates rates;
    float oreRemoved = 0.0f;
    float oreCollected = 0.0f;
    bool factoryActive = false;
};
}

void updateTileResourceRates(Tile& tile)
{
    storeRates(tile, calculateRates(tile));
}

void updateResources(GameState& state)
{
    constexpr int tileCount = boardColumns * boardRows;
    std::array<ResourcePlan, tileCount> plans{};
    std::array<int, tileCount> factories{};
    int factoryCount = 0;
    float projectedOre = state.ore;

    // Forecast every tile before changing trees, ground ore, or capital.
    for (int index = 0; index < tileCount; ++index) {
        const Tile& tile = state.tiles[index];
        ResourcePlan& plan = plans[index];
        plan.rates = calculateRates(tile);
        const float extraction = plan.rates.mineYield + plan.rates.mineWaste;
        if (extraction > 0.0f) {
            plan.oreRemoved = std::min(tile.ore, extraction);
            // On the last partial extraction, useful ore and waste shrink
            // proportionally. Factories can only spend the useful portion.
            plan.oreCollected = plan.oreRemoved * (plan.rates.mineYield / extraction);
            projectedOre += plan.oreCollected;
        }
        if (tile.type == TileType::Plains && tile.content == TileContent::Factory) {
            factories[factoryCount++] = index;
        }
    }

    std::sort(factories.begin(), factories.begin() + factoryCount,
        [&plans](int left, int right) {
            const auto& a = plans[left].rates;
            const auto& b = plans[right].rates;
            const float outputA = a.throughput * a.efficiency;
            const float outputB = b.throughput * b.efficiency;
            return outputA == outputB ? left < right : outputA > outputB;
        });
    float capitalProduced = 0.0f;
    for (int order = 0; order < factoryCount; ++order) {
        ResourcePlan& plan = plans[factories[order]];
        if (plan.rates.throughput > 0.0f && plan.rates.efficiency > 0.0f
            && projectedOre - plan.rates.throughput >= config::factoryOreReserve) {
            plan.factoryActive = true;
            projectedOre -= plan.rates.throughput;
            capitalProduced += plan.rates.throughput * plan.rates.efficiency;
        }
    }

    state.oreGainPerUpdate = projectedOre - state.ore;
    state.capitalGainPerUpdate = capitalProduced;
    state.ore = projectedOre;
    state.liquidCapital += capitalProduced;
    for (int index = 0; index < tileCount; ++index) {
        Tile& tile = state.tiles[index];
        const ResourcePlan& plan = plans[index];
        if (tile.type == TileType::Plains) {
            switch (tile.content) {
            case TileContent::None: break;
            case TileContent::Forest:
                tile.trees = std::clamp(tile.trees + plan.rates.treeChange,
                                        0.0f, config::forestTreeLimit);
                if (tile.trees == 0.0f) tile.content = TileContent::None;
                break;
            case TileContent::OpenPitMine:
                tile.ore = std::max(0.0f, tile.ore - plan.oreRemoved);
                if (tile.ore == 0.0f) tile.alive = false;
                break;
            case TileContent::Factory:
                tile.alive = plan.factoryActive;
                break;
            }
        }
        storeRates(tile, plan.rates);
        if (tile.content == TileContent::None) tile.treesDecayRate = 0.0f;
    }

    // Use post-production trees/ground ore, but pre-pollution pollution.
    // Every neighborhood reads the same snapshot, so traversal order cannot
    // spread freshly produced pollution twice in a single resource tick.
    const auto pollutionSnapshot = state.tiles;
    for (int row = 0; row < boardRows; ++row) {
        for (int column = 0; column < boardColumns; ++column) {
            const int index = row * boardColumns + column;
            const Tile& before = pollutionSnapshot[index];
            float pollutionChange = 0.0f;
            if (plans[index].rates.mineYield + plans[index].rates.mineWaste > 0.0f) {
                // A mine emits on its final extraction tick, then stops.
                pollutionChange += std::max(0.0f, before.initialOre - before.ore)
                    * config::minePollutionPerOreRemoved;
            }
            if (plans[index].factoryActive) {
                pollutionChange += plans[index].rates.throughput
                    * config::factoryPollutionPerOreProcessed;
            }
            const int radius = config::pollutionInteractionRadius;
            for (int nearbyRow = std::max(0, row - radius);
                 nearbyRow <= std::min(boardRows - 1, row + radius); ++nearbyRow) {
                for (int nearbyColumn = std::max(0, column - radius);
                     nearbyColumn <= std::min(boardColumns - 1, column + radius); ++nearbyColumn) {
                    const Tile& nearby = pollutionSnapshot[nearbyRow * boardColumns + nearbyColumn];
                    pollutionChange += nearby.pollution * config::nearbyPollutionGainPerUnit;
                    if (nearby.type == TileType::Plains && nearby.content == TileContent::Forest) {
                        pollutionChange -= nearby.trees * config::nearbyTreePollutionReductionPerTree;
                    }
                }
            }
            Tile& tile = state.tiles[index];
            tile.pollution = std::clamp(before.pollution + pollutionChange,
                                        config::minimumPollution, config::maximumPollution);
            tile.pollutionGainRate = tile.pollution - before.pollution;
        }
    }
}
}
