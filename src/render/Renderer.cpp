#include "render/Renderer.h"
#include "render/Animation.h"
#include "app/Config.h"
#include "game/GameState.h"
#include "input/Input.h"
#include "raylib.h"

#include <cmath>
#include <algorithm>
#include <sstream>
#include <string>

namespace game {
Renderer::TerrainAnimation Renderer::loadTerrain(const char* filename)
{
    // Desktop assets live beside the executable; web assets are preloaded.
#ifdef __EMSCRIPTEN__
    const char* path = TextFormat("assets/%s", filename);
#else
    const char* path = TextFormat("%sassets/%s", GetApplicationDirectory(), filename);
#endif
    TerrainAnimation result;
    Image image = LoadImageAnim(path, &result.frameCount);
    if (image.data != nullptr && result.frameCount > 0) {
        result.frameHeight = image.height;
        // LoadImageAnim returns consecutive RGBA frames. Increasing height
        // presents that buffer as a vertical atlas without copying pixels.
        image.height *= result.frameCount;
        result.texture = LoadTextureFromImage(image);
        if (result.texture.id != 0) {
            SetTextureFilter(result.texture, TEXTURE_FILTER_POINT);
        }
    }
    UnloadImage(image); // Only the GPU atlas is retained.
    return result;
}

namespace {
constexpr int uiFontSize = 24;
constexpr Color counterColor{0xc5, 0xc4, 0xcc, 255};

void drawCenteredText(const char* text, int y, int fontSize, Color color = RAYWHITE)
{
    DrawText(text, (config::windowWidth - MeasureText(text, fontSize)) / 2,
             y, fontSize, color);
}

Color tileColor(TileType type)
{
    switch (type) {
    case TileType::Plains: return Color{150, 162, 112, 255};
    case TileType::Mountain: return Color{100, 105, 112, 255};
    }
    return MAGENTA; // Makes an unexpected value visible during development.
}

void drawTutorial(const char* text, int x, bool visible)
{
    if (!visible || text[0] == '\0') return;
    const int lineHeight = config::tutorialFontSize + config::tutorialLineSpacing;
    const Color color = GetColor(config::selectionTextColor);
    int y = config::tutorialTop;
    BeginScissorMode(x, y, config::tutorialWidth, config::tutorialHeight);
    std::istringstream paragraphs(text);
    std::string paragraph;
    while (std::getline(paragraphs, paragraph)) {
        std::istringstream words(paragraph);
        std::string word;
        std::string line;
        while (words >> word) {
            const std::string candidate = line.empty() ? word : line + " " + word;
            if (!line.empty() && MeasureText(candidate.c_str(), config::tutorialFontSize) > config::tutorialWidth) {
                DrawText(line.c_str(), x, y, config::tutorialFontSize, color);
                y += lineHeight;
                line = word;
            } else {
                line = candidate;
            }
        }
        DrawText(line.c_str(), x, y, config::tutorialFontSize, color);
        y += lineHeight;
    }
    EndScissorMode();
    DrawText("dismiss", x, config::tutorialDismissY, config::tutorialFontSize, color);
}
}

Renderer::Renderer()
    : plains_(loadTerrain("Plains.gif")),
      forest_(loadTerrain("PinkTree.gif")),
      mountain_(loadTerrain("Mountain.gif")),
      liveFactory_(loadTerrain("LiveFactory.gif")),
      deadFactory_(loadTerrain("DeadFactory.gif")),
      liveOpenPitMine_(loadTerrain("LiveOpenPitMine.gif")),
      deadOpenPitMine_(loadTerrain("DeadOpenPitMine.gif"))
{
}

Renderer::~Renderer()
{
    if (plains_.texture.id != 0) UnloadTexture(plains_.texture);
    if (forest_.texture.id != 0) UnloadTexture(forest_.texture);
    if (mountain_.texture.id != 0) UnloadTexture(mountain_.texture);
    if (liveFactory_.texture.id != 0) UnloadTexture(liveFactory_.texture);
    if (deadFactory_.texture.id != 0) UnloadTexture(deadFactory_.texture);
    if (liveOpenPitMine_.texture.id != 0) UnloadTexture(liveOpenPitMine_.texture);
    if (deadOpenPitMine_.texture.id != 0) UnloadTexture(deadOpenPitMine_.texture);
}

void Renderer::drawSprite(const TerrainAnimation& sprite, int x, int y, int size,
                          double seconds, bool pingPong) const
{
    if (sprite.texture.id == 0) return;
    const int frame = pingPong
        ? animation::pingPongFrame(seconds, sprite.frameCount)
        : animation::loopingFrame(seconds, sprite.frameCount);
    DrawTexturePro(sprite.texture,
        {0.0f, static_cast<float>(frame * sprite.frameHeight),
         static_cast<float>(sprite.texture.width), static_cast<float>(sprite.frameHeight)},
        {static_cast<float>(x), static_cast<float>(y),
         static_cast<float>(size), static_cast<float>(size)},
        {0.0f, 0.0f}, 0.0f, WHITE);
}

void Renderer::drawTile(const Tile& tile, int x, int y, int size, double seconds) const
{
    DrawRectangle(x, y, size, size, tileColor(tile.type));
    drawSprite(tile.type == TileType::Mountain ? mountain_ : plains_, x, y, size, seconds);
    if (tile.type == TileType::Plains) {
        switch (tile.content) {
        case TileContent::None: break;
        case TileContent::Forest: drawSprite(forest_, x, y, size, seconds, true); break;
        case TileContent::Factory:
            drawSprite(tile.alive ? liveFactory_ : deadFactory_, x, y, size, seconds);
            break;
        case TileContent::OpenPitMine:
            drawSprite(tile.alive ? liveOpenPitMine_ : deadOpenPitMine_, x, y, size, seconds);
            break;
        }
    }
    DrawRectangleLines(x, y, size, size, BLACK);
}

void Renderer::drawSelection(const Tile& tile, int centerX, int centerY) const
{
    const int size = config::tileSize * config::selectionTileScale;
    const int top = centerY - size / 2;
    drawTile(tile, centerX - size / 2, top, size, selectionAnimationSeconds_);
    const int lineCount = tile.type == TileType::Mountain ? 2
        : tile.content == TileContent::None || tile.content == TileContent::Forest ? 5
        : tile.content == TileContent::OpenPitMine ? 4 : 3;
    const int textHeight = lineCount * (config::selectionFontSize + config::selectionLineSpacing)
        - config::selectionLineSpacing;
    int y = top + size + config::selectionTextGap;
    const int counterTop = config::windowHeight - uiFontSize - 20;
    if (y + textHeight > counterTop - config::selectionTextGap) {
        y = top - config::selectionTextGap - textHeight;
    }
    const auto line = [&y, centerX](const char* text, bool pollution = false) {
        DrawText(text, centerX - MeasureText(text, config::selectionFontSize) / 2,
                 y, config::selectionFontSize,
                 GetColor(pollution ? config::pollutionTextColor : config::selectionTextColor));
        y += config::selectionFontSize + config::selectionLineSpacing;
    };
    if (tile.type == TileType::Mountain) {
        line(TextFormat("Pollution: %.2f", tile.pollution), true);
        line(TextFormat("Pollution growth rate: %.2f", tile.pollutionGainRate), true);
        return;
    }
    switch (tile.content) {
    case TileContent::None:
        line(TextFormat("[F]ACTORY %.2f capital %.2f ore",
                        config::factoryCapitalCost, config::factoryOreCost));
        line(TextFormat("[M]INE %.2f capital %.2f ore",
                        config::openPitMineCapitalCost, config::openPitMineOreCost));
        line(TextFormat("Ore: %.2f", tile.ore));
        line(TextFormat("Pollution: %.2f", tile.pollution), true);
        line(TextFormat("Pollution growth rate: %.2f", tile.pollutionGainRate), true);
        break;
    case TileContent::Forest:
        line(TextFormat("Trees: %.2f", tile.trees));
        line(TextFormat("Ore: %.2f", tile.ore));
        line(TextFormat("Pollution: %.2f", tile.pollution), true);
        line(TextFormat("Tree %s rate: %.2f", tile.treesDecayRate < 0.0f ? "decay" : "growth",
                        std::fabs(tile.treesDecayRate)));
        line(TextFormat("Pollution growth rate: %.2f", tile.pollutionGainRate), true);
        break;
    case TileContent::OpenPitMine:
        line(TextFormat("Pollution: %.2f", tile.pollution), true);
        line(TextFormat("Pollution growth rate: %.2f", tile.pollutionGainRate), true);
        line(TextFormat("Ore: %.2f", tile.ore));
        line(TextFormat("Ore mine rate: %.2f", tile.oreMineRate));
        break;
    case TileContent::Factory:
        line(TextFormat("Pollution: %.2f", tile.pollution), true);
        line(TextFormat("Pollution growth rate: %.2f", tile.pollutionGainRate), true);
        line(TextFormat("%.2f ore -> %.2f capital per update",
                        tile.throughput, tile.throughput * tile.efficiency));
        break;
    }
}

void Renderer::advanceAnimation(float deltaSeconds, int selectedTile)
{
    if (selectedTile >= 0) {
        if (animatedSelection_ != selectedTile) {
            selectionAnimationSeconds_ = animationSeconds_;
        }
        selectionAnimationSeconds_ += deltaSeconds;
    } else {
        animationSeconds_ += deltaSeconds;
    }
    animatedSelection_ = selectedTile;
}

void Renderer::drawGame(const GameState& state, const InputState& input) const
{
    BeginDrawing();
    ClearBackground(state.selectedTile >= 0 ? GetColor(config::selectionBackgroundColor) : BLACK);
    if (state.scene == Scene::Title) {
        drawCenteredText("Click to begin", config::windowHeight / 2 - uiFontSize / 2, uiFontSize);
    } else if (state.scene == Scene::End) {
        drawCenteredText("You won!", config::windowHeight / 2 - uiFontSize - 10, uiFontSize);
        drawCenteredText("Click to start over", config::windowHeight / 2 + 10, uiFontSize);
    } else {
        drawTutorial(config::leftTutorialText, config::tutorialLeftX, state.leftTutorialVisible);
        drawTutorial(config::rightTutorialText, config::tutorialRightX, state.rightTutorialVisible);
        const int left = (config::windowWidth - boardColumns * config::tileSize) / 2;
        const int top = (config::windowHeight - boardRows * config::tileSize) / 2;
        const Vector2 mouse = GetMousePosition();
        for (int row = 0; row < boardRows; ++row) {
            for (int column = 0; column < boardColumns; ++column) {
                const int x = left + column * config::tileSize;
                const int y = top + row * config::tileSize;
                const Tile& tile = state.tiles[row * boardColumns + column];
                drawTile(tile, x, y, config::tileSize, animationSeconds_);
                if (state.selectedTile < 0) {
                    Color tint{0, 0, 0, 0};
                    // Keep map modes distinct when several keys are held: I > O > P.
                    if (input.pollutionGrowthOverlayHeld) {
                        if (tile.pollutionGainRate != 0.0f) {
                            tint = GetColor(tile.pollutionGainRate > 0.0f
                                ? config::pollutionGrowthPositiveColor : config::pollutionGrowthNegativeColor);
                            tint.a = config::pollutionGrowthOverlayOpacity;
                        }
                    } else if (input.oreOverlayHeld) {
                        const float intensity = tile.initialOre > 0.0f
                            ? std::clamp(tile.ore / tile.initialOre, 0.0f, 1.0f) : 0.0f;
                        tint = GetColor(config::oreOverlayColor);
                        tint.a = static_cast<unsigned char>(std::lround(intensity * config::oreOverlayMaxOpacity));
                    } else if (input.pollutionOverlayHeld) {
                        const float intensity = std::clamp(tile.pollution / config::maximumPollution,
                                                           0.0f, 1.0f);
                        tint = GetColor(config::pollutionOverlayColor);
                        tint.a = static_cast<unsigned char>(std::lround(intensity * config::pollutionOverlayMaxOpacity));
                    }
                    if (tint.a > 0) DrawRectangle(x, y, config::tileSize, config::tileSize, tint);
                }
                if (state.selectedTile < 0
                    && mouse.x >= x && mouse.x < x + config::tileSize
                    && mouse.y >= y && mouse.y < y + config::tileSize) {
                    DrawRectangle(x, y, config::tileSize, config::tileSize,
                                  Color{255, 255, 255, config::tileHoverOpacity});
                }
            }
        }
        if (state.selectedTile >= 0) {
            const int centerX = left + (state.selectedTile % boardColumns) * config::tileSize
                + config::tileSize / 2;
            const int centerY = top + (state.selectedTile / boardColumns) * config::tileSize
                + config::tileSize / 2;
            // Draw last so the enlarged tile sits above the frozen board.
            drawSelection(state.tiles[state.selectedTile], centerX, centerY);
        }
        const char* counters = TextFormat(
            "Ore : %.0f [%+.2f]     Liquid Capital : %.0f [%+.2f]     Total Capital : %.0f [%+.2f]",
            state.ore, state.oreGainPerUpdate, state.liquidCapital, state.capitalGainPerUpdate,
            state.liquidCapital + state.buildingCapital, state.capitalGainPerUpdate);
        drawCenteredText(counters, config::windowHeight - uiFontSize - 20,
                         uiFontSize, counterColor);
        if (state.insufficientResourcesRemaining > 0.0f) {
            drawCenteredText("INSUFFICIENT RESOURCES",
                             (config::windowHeight - config::selectionFontSize) / 2,
                             config::selectionFontSize, WHITE);
        }
    }
    EndDrawing();
}
}
