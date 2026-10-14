#pragma once

namespace game {
// A snapshot of this frame's input; no raylib types enter game state.
struct InputState {
    bool primaryPressed = false;
    float mouseX = 0.0f;
    float mouseY = 0.0f;
    bool anyMousePressed = false;
    bool anyKeyPressed = false;
    bool factoryPressed = false;
    bool minePressed = false;
    bool pollutionOverlayHeld = false;
    bool oreOverlayHeld = false;
    bool pollutionGrowthOverlayHeld = false;
    bool dismissLeftTutorial = false;
    bool dismissRightTutorial = false;
};

InputState readInput();
}
