#pragma once

namespace game {
// A snapshot of this frame's input; no raylib types enter game state.
struct InputState {
    bool primaryPressed = false;
    float mouseX = 0.0f;
    float mouseY = 0.0f;
};

InputState readInput();
}
