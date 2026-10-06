#include "app/GameLoop.h"
#include "app/Config.h"
#include "game/GameState.h"
#include "input/Input.h"
#include "render/Renderer.h"
#include "raylib.h"

#include <algorithm>
#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

namespace game {
namespace {
// The callback receives a borrowed pointer; it does not own the state.
void frame(void* context)
{
    auto& state = *static_cast<GameState*>(context);
    const InputState input = readInput();
    // Avoid a huge simulation jump after pausing or moving the window.
    const float deltaSeconds = std::min(GetFrameTime(), 0.1f);
    updateGame(state, input, deltaSeconds);
    drawGame(state);
}
}

int run()
{
    InitWindow(config::windowWidth, config::windowHeight, config::windowTitle);
    SetTargetFPS(config::targetFps);

#ifdef __EMSCRIPTEN__
    // Browser callbacks outlive this stack frame, so state has static lifetime.
    static GameState state = makeInitialGameState();
    emscripten_set_main_loop_arg(frame, &state, 0, true);
#else
    GameState state = makeInitialGameState();
    while (!WindowShouldClose()) {
        frame(&state);
    }
    CloseWindow();
#endif
    return 0;
}
}
