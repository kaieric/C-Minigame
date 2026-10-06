#include "game/GameState.h"
#include "input/Input.h"

namespace game {
GameState makeInitialGameState()
{
    // All tiles start as plains. Put your deterministic map layout here.
    return GameState{};
}

void updateGame(GameState& state, const InputState& input, float deltaSeconds)
{
    if (input.primaryPressed && state.scene != Scene::Playing) {
        state = makeInitialGameState();
        state.scene = Scene::Playing;
        return;
    }
    if (state.scene != Scene::Playing) {
        return;
    }

    // Your next steps: building placement, forest regeneration, harvesting,
    // factory production, pollution, then victory detection.
    // Rates should use seconds: wood += regenerationPerSecond * deltaSeconds.
    (void)deltaSeconds; // Intentionally unused until simulation is added.
}
}
