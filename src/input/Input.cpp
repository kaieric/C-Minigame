#include "input/Input.h"
#include "app/Config.h"
#include "raylib.h"

namespace game {
InputState readInput()
{
    const Vector2 mouse = GetMousePosition();
    InputState input;
    input.primaryPressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    input.mouseX = mouse.x;
    input.mouseY = mouse.y;
    input.pollutionOverlayHeld = IsKeyDown(KEY_P);
    input.oreOverlayHeld = IsKeyDown(KEY_O);
    input.pollutionGrowthOverlayHeld = IsKeyDown(KEY_I);
    const auto dismissClicked = [&input, mouse](int x, const char* text) {
        return input.primaryPressed && text[0] != '\0'
            && mouse.x >= x && mouse.x < x + MeasureText("dismiss", config::tutorialFontSize)
            && mouse.y >= config::tutorialDismissY
            && mouse.y < config::tutorialDismissY + config::tutorialFontSize;
    };
    input.dismissLeftTutorial = dismissClicked(config::tutorialLeftX, config::leftTutorialText);
    input.dismissRightTutorial = dismissClicked(config::tutorialRightX, config::rightTutorialText);
    for (int button = MOUSE_BUTTON_LEFT; button <= MOUSE_BUTTON_BACK; ++button) {
        input.anyMousePressed |= IsMouseButtonPressed(button);
    }
    // Only the first key chooses an action; drain the queue so keys cannot
    // carry over into a later selection.
    const int key = GetKeyPressed();
    input.anyKeyPressed = key != 0;
    input.factoryPressed = key == KEY_F;
    input.minePressed = key == KEY_M;
    while (GetKeyPressed() != 0) {}
    return input;
}
}
