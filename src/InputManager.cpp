#include "InputManager.hpp"
#include "structs.hpp"

InputManager::InputManager(GLFWwindow *win, int width, int height)
    : window(win), windowWidth(width), windowHeight(height), prevMouseLeftPressed(false) {}

void InputManager::updateMouseInput(GameStatus &p) {
    // Get mouse position in pixels
    glfwGetCursorPos(window, &mouseState.pixelX, &mouseState.pixelY);

    // Convert to NDC (Normalized Device Coordinates: -1 to +1)
    //   X: -1 (left) to +1 (right)
    //   Y: -1 (bottom) to +1 (top)
    mouseState.ndcX = (static_cast<float>(mouseState.pixelX) / windowWidth) * 2.0f - 1.0f;
    mouseState.ndcY = -(1.0f - (static_cast<float>(mouseState.pixelY) / windowHeight) * 2.0f);

    // Get current button state
    bool currPressed = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

    // Detect transitions
    mouseState.leftJustClicked = currPressed && !prevMouseLeftPressed;
    mouseState.leftJustReleased = !currPressed && prevMouseLeftPressed;

    // Store for next frame
    prevMouseLeftPressed = currPressed;

    Listenable<GameStatus &, MouseState>::update(p, mouseState);
}

void InputManager::updatePressedKeys(GameStatus &p) {
    std::ranges::for_each(keys, [this, &p](Key &k) {
        bool down = glfwGetKey(window, k.glfw_key);
        if (k.isDown && !down) {
            Listenable<GameStatus &, Key>::update(p, k);
            Listenable<Key>::update(k);
        }
        k.isDown = down;
    });
}

void InputManager::update(GameStatus &p, float deltaT) {
    updateMouseInput(p);
    updatePressedKeys(p);
}

void InputManager::onResize(int width, int height) {
    windowWidth = width;
    windowHeight = height;
}
