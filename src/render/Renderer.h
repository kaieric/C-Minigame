#pragma once

namespace game {
struct GameState;
// Const reference: rendering reads state and never changes gameplay.
void drawGame(const GameState& state);
}
