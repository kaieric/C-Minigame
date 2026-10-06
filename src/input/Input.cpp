#include "input/Input.h"
#include "raylib.h"

namespace game {
InputState readInput()
{
    const Vector2 mouse = GetMousePosition();
    return {IsMouseButtonPressed(MOUSE_BUTTON_LEFT), mouse.x, mouse.y};
}
}
