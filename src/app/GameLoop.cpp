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
struct FrameContext {
    GameState state = makeInitialGameState();
    Renderer renderer;
};
// The callback receives a borrowed pointer; it does not own the state.
void frame(void* context)
{
    auto& frameContext = *static_cast<FrameContext*>(context);
    auto& state = frameContext.state;
    const InputState input = readInput();
    // Avoid a huge simulation jump after pausing or moving the window.
    const float deltaSeconds = std::min(GetFrameTime(), 0.1f);
    updateGame(state, input, deltaSeconds);
    // Visual timing uses real elapsed time, independent of the simulation cap.
    if (state.scene == Scene::Playing) {
        frameContext.renderer.advanceAnimation(GetFrameTime(), state.selectedTile);
    }
    frameContext.renderer.drawGame(state, input);
}
}

int run()
{
    InitWindow(config::windowWidth, config::windowHeight, config::windowTitle);
    SetTargetFPS(config::targetFps);
    SetExitKey(KEY_NULL); // Escape dismisses a selection like any other key.

#ifdef __EMSCRIPTEN__
    // Browser callbacks outlive this stack frame, so state has static lifetime.
    static FrameContext context;
    emscripten_set_main_loop_arg(frame, &context, 0, true);
#else
    {
        FrameContext context;
        while (!WindowShouldClose()) {
            frame(&context);
        }
        // Renderer destructor unloads textures while the graphics context exists.
    }
    CloseWindow();
#endif
    return 0;
}
}
