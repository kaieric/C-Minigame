#pragma once

namespace game::config {
//Board
inline constexpr int boardColumns = 10;
inline constexpr int boardRows = 10;
inline constexpr int windowWidth = 1200;
inline constexpr int windowHeight = 675;
inline constexpr int targetFps = 60;
inline constexpr int tileSize = 32; //Sets the Square Tile Side Lengths. Images are scaled to this.
//Resources
inline constexpr float basePlainsOre = 270.0f;
inline constexpr float orePerAdjacentMountain = 900.0f;
inline constexpr float startingOre = 200.0f;
inline constexpr float startingLiquidCapital = 20.0f;
inline constexpr float winningTotalCapital = 500.0f; // Liquid capital plus fixed building capital.
//Buildings
// Capital cost is also the fixed value of each live or dead building.
inline constexpr float factoryCapitalCost = 10.0f;
inline constexpr float factoryOreCost = 20.0f;
inline constexpr float openPitMineCapitalCost = 4.0f;
inline constexpr float openPitMineOreCost = 10.0f;
// Resource updates and interactions (all rates are per resource update).
inline constexpr int resourceUpdateInterval = 180; // About 3 seconds at 60 FPS; selection pauses it.
inline constexpr float pollutionRateStep = 10.0f; // Pollution units used by the rate bonuses/penalties below.
inline constexpr float startingForestTrees = 120.0f;
inline constexpr float forestTreeLimit = 120.0f;
inline constexpr float forestBaseGrowth = 4.0f;
inline constexpr float forestGrowthLossPerPollutionStep = 1.0f;
inline constexpr float baseMineRate = 3.0f; // Usable ore added to the stockpile.
inline constexpr float acceleratedMineRate = 0.6f; // Extra usable ore per pollution step.
inline constexpr float oreWastagePerPollutionStep = 0.2f; // Extra ground ore removed, never collected.
inline constexpr float factoryThroughput = 50.0f; // Base ore consumption.
inline constexpr float factoryEfficiency = 0.05f; // Base capital produced per ore processed.
inline constexpr float factoryThroughputGainPerPollutionStep = 5.0f;
inline constexpr float factoryEfficiencyLossPerPollutionStep = 0.03f; // Linear fraction of base efficiency.
inline constexpr float factoryOreReserve = 3.0f * openPitMineOreCost; // Leave enough collected ore for three mines.
inline constexpr float minimumPollution = 0.0f;
inline constexpr float maximumPollution = 100.0f;
inline constexpr float minePollutionPerOreRemoved = 0.005f; // 0.05 per 10 cumulatively removed ore, including waste.
inline constexpr float factoryPollutionPerOreProcessed = 0.2f; // 1 pollution per 5 ore processed this update.
inline constexpr int pollutionInteractionRadius = 1; // 3x3 neighborhood, including this tile.
inline constexpr float nearbyPollutionGainPerUnit = 0.005f; // +0.1 per 20 total nearby pollution.
inline constexpr float nearbyTreePollutionReductionPerTree = 0.005f; // -0.1 per 20 total nearby trees.
//Fonts and Colors
inline constexpr int selectionFontSize = 15;
inline constexpr int selectionTileScale = 2;
inline constexpr unsigned int selectionBackgroundColor = 0x212028ff;
inline constexpr unsigned int selectionTextColor = 0xc5c4ccff;
inline constexpr unsigned int pollutionTextColor = 0xff5555ff;
inline constexpr unsigned int pollutionOverlayColor = 0xff0000ff; // Red tint; opacity is set per tile.
inline constexpr unsigned char pollutionOverlayMaxOpacity = 80; // Alpha at maximum pollution, 0–255.
inline constexpr unsigned int oreOverlayColor = 0x808080ff;
inline constexpr unsigned char oreOverlayMaxOpacity = 80; // Alpha at the tile's full original ore reserve.
inline constexpr unsigned int pollutionGrowthPositiveColor = 0xff0000ff;
inline constexpr unsigned int pollutionGrowthNegativeColor = 0x00ff00ff;
inline constexpr unsigned char pollutionGrowthOverlayOpacity = 80; // Fixed alpha for any nonzero change.
inline constexpr unsigned char tileHoverOpacity = 32; // White overlay alpha, 0–255.
inline constexpr int selectionTextGap = 12;
inline constexpr int selectionLineSpacing = 8;
inline constexpr float insufficientResourcesSeconds = 2.0f;
// Side tutorials: edit either text; use \n for explicit line breaks or "" to hide it.
inline constexpr const char* leftTutorialText =
    "Click a tile to inspect it. This freezes time.\n\n"
    "Build on empty plains with F for a factory or M for a mine.\n\n"
    "Mines collect ore. Factories turn ore into capital. Press O to see ore remaining in the ground";
inline constexpr const char* rightTutorialText =
    "Mines and Factories Produce Pollution. Forests remove it. Hold P to view Pollution\n\n"
    "Pollution accelerates industry but harms efficiency and forests. Press I to show pollution growth rates.\n\n"
    "More ore is located near mountains.";
inline constexpr int tutorialFontSize = 16;
inline constexpr int tutorialLineSpacing = 6;
inline constexpr int tutorialLeftX = 40;
inline constexpr int tutorialRightX = 860;
inline constexpr int tutorialTop = 170;
inline constexpr int tutorialWidth = 300;
inline constexpr int tutorialHeight = 220;
inline constexpr int tutorialDismissY = tutorialTop + tutorialHeight + 12;
inline constexpr const char* windowTitle = "Finite Yield";
}
