#include "app/Config.h"
#include "game/GameState.h"
#include "game/ResourceUpdate.h"
#include "input/Input.h"

#include <cmath>
#include <cstdlib>
#include <iostream>

namespace {
using namespace game;

void expect(bool condition, const char* message)
{
    if (!condition) {
        std::cerr << message << '\n';
        std::exit(1);
    }
}

void near(float actual, float expected, const char* message)
{
    if (std::fabs(actual - expected) > 0.0001f) {
        std::cerr << message << ": expected " << expected << ", got " << actual << '\n';
        std::exit(1);
    }
}

GameState blankGame()
{
    GameState state;
    state.scene = Scene::Playing;
    return state;
}

Tile forest(float trees, float pollution)
{
    Tile tile{TileType::Plains, TileContent::Forest};
    tile.trees = trees;
    tile.pollution = pollution;
    return tile;
}

Tile mine(float ore, float pollution = 0.0f)
{
    Tile tile{TileType::Plains, TileContent::OpenPitMine};
    tile.ore = ore;
    tile.initialOre = 90.0f;
    tile.pollution = pollution;
    return tile;
}

Tile factory(float pollution = 0.0f)
{
    Tile tile{TileType::Plains, TileContent::Factory};
    tile.pollution = pollution;
    return tile;
}

void testForests()
{
    for (const float pollution : {0.0f, 20.0f, 40.0f, 100.0f}) {
        auto state = blankGame();
        state.tiles[12] = forest(80.0f, pollution);
        updateResources(state);
        const float expected = pollution == 0 ? 84 : pollution == 20 ? 82 : pollution == 40 ? 80 : 74;
        near(state.tiles[12].trees, expected, "forest growth uses starting pollution");
    }
    auto state = blankGame();
    state.tiles[12] = forest(119.0f, 0.0f);
    updateResources(state);
    near(state.tiles[12].trees, 120.0f, "forest tree limit");
    state.tiles[12] = forest(5.0f, 100.0f);
    updateResources(state);
    expect(state.tiles[12].content == TileContent::None, "zero-tree forest must disappear");
    near(state.tiles[12].trees, 0.0f, "tree count cannot become negative");
    near(state.tiles[12].treesDecayRate, 0.0f, "removed forest has no growth rate");
}

void testMines()
{
    auto state = blankGame();
    state.tiles[12] = mine(90.0f, 20.0f);
    updateResources(state);
    near(state.ore, 4.2f, "polluted mine usable output");
    near(state.oreGainPerUpdate, 4.2f, "ore counter records usable production per tick");
    near(state.tiles[12].ore, 85.4f, "mining removes usable output and wastage");
    near(state.tiles[12].oreMineRate, 4.6f, "displayed extraction includes wastage");
    near(state.tiles[12].oreWasteRate, 0.4f, "displayed wastage");
    near(state.tiles[12].pollution, 20.123f, "mining pollution includes this extraction and old self pollution");

    state = blankGame();
    state.tiles[12] = mine(1.0f, 20.0f);
    updateResources(state);
    near(state.ore, 0.9130435f, "partial last extraction preserves useful/waste ratio");
    near(state.tiles[12].ore, 0.0f, "mine ground ore is exhausted");
    expect(!state.tiles[12].alive, "exhausted mine dies");
    near(state.tiles[12].oreMineRate, 0.0f, "dead mine has no extraction rate");
    const float collected = state.ore;
    updateResources(state);
    near(state.ore, collected, "dead mine never produces more ore");
    near(state.oreGainPerUpdate, 0.0f, "ore counter resets when production stops");

    state = blankGame();
    state.tiles[12] = mine(43.0f);
    updateResources(state);
    near(state.tiles[12].ore, 40.0f, "cumulative mining example remaining reserve");
    near(state.tiles[12].pollutionGainRate, 0.25f, "40/90 remaining emits 0.25 pollution");
    state = blankGame();
    state.tiles[12] = mine(90.0f);
    state.tiles[12].alive = false;
    updateResources(state);
    near(state.ore, 0.0f, "already dead mine does not restart");
}

void testFactories()
{
    auto state = blankGame();
    state.ore = config::factoryOreReserve + config::factoryThroughput + 10.0f;
    state.tiles[0] = factory();
    state.tiles[boardRows * boardColumns - 1] = factory(20.0f);
    updateResources(state);
    expect(!state.tiles[0].alive && state.tiles[boardRows * boardColumns - 1].alive, "higher capital output runs first regardless of board order");
    near(state.ore, config::factoryOreReserve, "factory preserves ore for one mine");
    near(state.liquidCapital, (config::factoryThroughput + 10.0f) * 0.047f, "factory efficiency loses 3 percent of base per pollution step");
    near(state.oreGainPerUpdate, -(config::factoryThroughput + 10.0f), "ore production counter includes factory consumption");
    near(state.capitalGainPerUpdate, (config::factoryThroughput + 10.0f) * 0.047f, "capital production counter records produced capital");
    near(state.tiles[boardRows * boardColumns - 1].throughput, config::factoryThroughput + 10.0f, "polluted factory throughput");
    near(state.tiles[boardRows * boardColumns - 1].efficiency, 0.047f, "polluted factory efficiency");
    near(state.tiles[0].throughput, 0.0f, "inactive factory reports zero consumption");
    state.ore = config::factoryOreReserve + 4.0f * config::factoryThroughput + 40.0f;
    updateResources(state);
    expect(state.tiles[0].alive, "inactive factory can restart next tick");

    state = blankGame();
    state.ore = config::factoryOreReserve + config::factoryThroughput;
    state.tiles[0] = factory();
    state.tiles[boardRows * boardColumns - 1] = factory(20.0f);
    updateResources(state);
    expect(state.tiles[0].alive && !state.tiles[boardRows * boardColumns - 1].alive, "skip unaffordable high-output factory but try cheaper ones");
    near(state.ore, config::factoryOreReserve, "exact reserve boundary is affordable");
    near(state.liquidCapital, config::factoryThroughput * config::factoryEfficiency, "base factory capital output");

    state = blankGame();
    state.ore = config::factoryOreReserve + 2.0f * config::factoryThroughput;
    state.tiles[0] = factory();
    state.tiles[boardRows * boardColumns - 1] = factory();
    updateResources(state);
    near(state.ore, config::factoryOreReserve, "reserve check accounts for earlier activations");
    near(state.liquidCapital, 2.0f * config::factoryThroughput * config::factoryEfficiency, "both factories produce when affordable");
    near(state.buildingCapital, 0.0f, "production does not change fixed building capital");
}

void testMineForecast()
{
    auto state = blankGame();
    state.ore = config::factoryOreReserve + config::factoryThroughput - 3.0f;
    state.tiles[0] = factory();
    state.tiles[boardRows * boardColumns - 1] = mine(90.0f);
    updateResources(state);
    expect(state.tiles[0].alive, "factory can use mine output forecast for this tick");
    near(state.ore, config::factoryOreReserve, "forecast production funds factory and reserve");
    near(state.oreGainPerUpdate, 3.0f - config::factoryThroughput, "net ore counter combines mining and factory consumption");

    state = blankGame();
    state.ore = config::factoryOreReserve + config::factoryThroughput - 3.0f;
    state.tiles[0] = factory();
    state.tiles[boardRows * boardColumns - 1] = mine(1.0f);
    updateResources(state);
    expect(!state.tiles[0].alive, "forecast must cap production at remaining ground ore");
    near(state.ore, config::factoryOreReserve + config::factoryThroughput - 2.0f, "partial final mine output is available");

    state = blankGame();
    state.ore = config::factoryOreReserve + config::factoryThroughput - 1.0f;
    state.tiles[0] = factory();
    state.tiles[boardRows * boardColumns - 1] = mine(1.0f, 20.0f);
    updateResources(state);
    expect(!state.tiles[0].alive, "forecast must exclude wastage even on final extraction");
    near(state.ore, config::factoryOreReserve + config::factoryThroughput - 0.0869565f, "wasted final ore never enters stockpile");
}

void testPollution()
{
    auto state = blankGame();
    state.ore = config::factoryOreReserve + config::factoryThroughput;
    state.tiles[12] = factory();
    updateResources(state);
    near(state.tiles[12].pollution, config::factoryThroughput * 0.2f, "factory adds 1 pollution per 5 processed ore");
    near(state.tiles[11].pollution, 0.0f, "new source pollution does not spread in same tick");
    updateResources(state);
    near(state.tiles[11].pollution, config::factoryThroughput * 0.001f, "previous source pollution affects neighbors next tick");
    near(state.tiles[12].pollution, config::factoryThroughput * 0.201f, "inactive factory only gains neighborhood pollution");

    state = blankGame();
    for (auto& tile : state.tiles) tile.pollution = 20.0f;
    state.tiles[0] = forest(40.0f, 20.0f);
    updateResources(state);
    near(state.tiles[0].trees, 42.0f, "trees grow before pollution calculation");
    near(state.tiles[0].pollution, 20.19f, "corner neighborhood includes self and updated trees");
    near(state.tiles[1].pollution, 20.39f, "edge neighborhood contains six tiles");
    near(state.tiles[12].pollution, 20.9f, "interior neighborhood contains nine tiles");
    near(state.tiles[boardRows * boardColumns - 1].pollution, 20.4f, "far corner excludes distant forest");

    state = blankGame();
    state.tiles[12] = forest(120.0f, 0.0f);
    updateResources(state);
    near(state.tiles[12].pollution, 0.0f, "tree absorption cannot make pollution negative");
    near(state.tiles[12].pollutionGainRate, 0.0f, "displayed pollution change respects lower bound");
    for (auto& tile : state.tiles) tile.pollution = 100.0f;
    updateResources(state);
    near(state.tiles[boardRows * boardColumns - 1].pollution, 100.0f, "pollution cannot exceed upper bound");

    state = blankGame();
    for (auto& tile : state.tiles) tile.pollution = 20.0f;
    state.tiles[0] = forest(1.0f, 100.0f);
    updateResources(state);
    near(state.tiles[1].pollution, 21.0f, "removed forest no longer absorbs neighborhood pollution");
}

void testSchedulingAndInitialization()
{
    const auto initial = makeInitialGameState();
    for (const auto& tile : initial.tiles) {
        if (tile.content == TileContent::Forest) {
            near(tile.trees, config::startingForestTrees, "forests start at configured tree count");
            near(tile.treesDecayRate, config::forestBaseGrowth, "initial forest rate is visible before first tick");
        }
        near(tile.initialOre, tile.ore, "original ore reserve recorded at initialization");
    }
    auto state = blankGame();
    state.tiles[0] = forest(80.0f, 0.0f);
    for (int frame = 0; frame < config::resourceUpdateInterval - 1; ++frame) {
        updateGame(state, {}, 1.0f / config::targetFps);
    }
    near(state.tiles[0].trees, 80.0f, "no resource tick before configured game-update count");
    InputState click;
    click.primaryPressed = click.anyMousePressed = true;
    click.mouseX = (config::windowWidth - boardColumns * config::tileSize) / 2 + config::tileSize / 2;
    click.mouseY = (config::windowHeight - boardRows * config::tileSize) / 2 + config::tileSize / 2;
    updateGame(state, click, 0.016f);
    expect(state.selectedTile == 0, "select forest immediately before pending resource tick");
    for (int frame = 0; frame < config::resourceUpdateInterval * 2; ++frame) updateGame(state, {}, 0.016f);
    near(state.tiles[0].trees, 80.0f, "selection pauses resource simulation");
    expect(state.updatesSinceResources == config::resourceUpdateInterval - 1, "selection preserves pending update count");
    InputState dismiss;
    dismiss.anyKeyPressed = true;
    updateGame(state, dismiss, 0.016f);
    near(state.tiles[0].trees, 80.0f, "dismissal frame stays paused");
    updateGame(state, {}, 0.016f);
    near(state.tiles[0].trees, 84.0f, "resource tick resumes after selection");
    expect(state.updatesSinceResources == 0, "resource counter resets after tick");
}

void testLongRunBounds()
{
    auto state = makeInitialGameState();
    for (int tick = 0; tick < 1000; ++tick) {
        updateResources(state);
        expect(std::isfinite(state.ore) && state.ore >= 0.0f, "stockpile remains finite and nonnegative");
        expect(std::isfinite(state.liquidCapital), "capital remains finite");
        for (const auto& tile : state.tiles) {
            expect(tile.ore >= 0.0f, "ground ore remains nonnegative");
            expect(tile.trees >= 0.0f && tile.trees <= config::forestTreeLimit, "tree count stays within limits");
            expect(tile.pollution >= config::minimumPollution && tile.pollution <= config::maximumPollution,
                   "pollution stays within limits");
        }
    }
}

void testBoardAndTutorials()
{
    auto state = makeInitialGameState();
    expect(boardColumns == 10 && boardRows == 10 && state.tiles.size() == 100, "board is 10x10");
    int factories = 0;
    int mines = 0;
    int topLeftTerrain = 0;
    int topRightTerrain = 0;
    int bottomRightTerrain = 0;
    for (int row = 0; row < boardRows; ++row) {
        for (int column = 0; column < boardColumns; ++column) {
            const auto& tile = state.tiles[row * boardColumns + column];
            factories += tile.content == TileContent::Factory;
            mines += tile.content == TileContent::OpenPitMine;
            const bool terrain = tile.type == TileType::Mountain || tile.content == TileContent::Forest;
            if (row < 5 && column < 5) topLeftTerrain += terrain;
            if (row < 5 && column >= 5) topRightTerrain += terrain;
            if (row >= 5 && column >= 5) bottomRightTerrain += terrain;
        }
    }
    expect(factories == 1 && mines == 2, "map starts with one factory and two mines");
    expect(state.tiles[90].content == TileContent::Factory, "factory starts in bottom-left clearing");
    expect(state.tiles[80].content == TileContent::OpenPitMine
        && state.tiles[91].content == TileContent::OpenPitMine, "two mines start beside factory");
    expect(topLeftTerrain > topRightTerrain && bottomRightTerrain > topRightTerrain,
           "terrain is concentrated in top-left and bottom-right");
    near(state.buildingCapital, config::factoryCapitalCost + 2 * config::openPitMineCapitalCost,
         "new starting buildings contribute fixed capital");

    state.scene = Scene::Playing;
    state.selectedTile = 90;
    InputState dismiss;
    dismiss.primaryPressed = dismiss.anyMousePressed = true;
    dismiss.dismissLeftTutorial = true;
    updateGame(state, dismiss, 0.016f);
    expect(!state.leftTutorialVisible && state.rightTutorialVisible, "left tutorial dismisses independently");
    expect(state.selectedTile == 90, "tutorial UI click does not select or dismiss a board tile");
    dismiss.dismissLeftTutorial = false;
    dismiss.dismissRightTutorial = true;
    updateGame(state, dismiss, 0.016f);
    expect(!state.rightTutorialVisible, "right tutorial can be dismissed");
    state = makeInitialGameState();
    expect(state.leftTutorialVisible && state.rightTutorialVisible, "new game restores both tutorials");
}

void testWinAndRestart()
{
    auto state = blankGame();
    state.buildingCapital = 40.0f;
    state.liquidCapital = config::winningTotalCapital - state.buildingCapital - 0.01f;
    updateGame(state, {}, 0.016f);
    expect(state.scene == Scene::Playing, "below winning total remains in play");
    state.liquidCapital = config::winningTotalCapital - state.buildingCapital;
    state.selectedTile = 0;
    updateGame(state, {}, 0.016f);
    expect(state.scene == Scene::End && state.selectedTile == -1,
           "exact winning total includes building capital and clears selection");

    state = blankGame();
    const float output = config::factoryThroughput * config::factoryEfficiency;
    state.tiles[0] = factory();
    state.buildingCapital = config::factoryCapitalCost;
    state.liquidCapital = config::winningTotalCapital - state.buildingCapital - output / 2.0f;
    state.ore = config::factoryOreReserve + config::factoryThroughput;
    state.updatesSinceResources = config::resourceUpdateInterval - 1;
    updateGame(state, {}, 0.016f);
    expect(state.scene == Scene::End, "resource production crossing winning total ends game immediately");
    const float capitalAtWin = state.liquidCapital;
    const float oreAtWin = state.ore;
    for (int frame = 0; frame < config::resourceUpdateInterval + 1; ++frame) updateGame(state, {}, 0.016f);
    near(state.liquidCapital, capitalAtWin, "end screen stops production");
    near(state.ore, oreAtWin, "end screen stops ore consumption");

    state.leftTutorialVisible = state.rightTutorialVisible = false;
    InputState click;
    click.primaryPressed = click.anyMousePressed = true;
    updateGame(state, click, 0.016f);
    expect(state.scene == Scene::Playing && state.selectedTile == -1, "end screen click starts a fresh game");
    near(state.ore, config::startingOre, "restart restores starting ore");
    near(state.liquidCapital, config::startingLiquidCapital, "restart restores starting liquid capital");
    near(state.buildingCapital, config::factoryCapitalCost + 2.0f * config::openPitMineCapitalCost,
         "restart restores starting buildings");
    near(state.oreGainPerUpdate, 0.0f, "restart clears last production rate");
    expect(state.updatesSinceResources == 0 && state.leftTutorialVisible && state.rightTutorialVisible,
           "restart resets tick counter and tutorials");
}
}

int main()
{
    testForests();
    testMines();
    testFactories();
    testMineForecast();
    testPollution();
    testSchedulingAndInitialization();
    testLongRunBounds();
    testBoardAndTutorials();
    testWinAndRestart();
    std::cout << "Resource update checks passed.\n";
}
