#include "InputManager.hpp"

InputManager::InputManager(GLFWwindow *win, int width, int height)
    : window(win),
      windowWidth(width),
      windowHeight(height),
      prevMouseLeftPressed(false) {
}

void InputManager::updateMouseInput() {
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
}

void InputManager::update(float deltaT) {
    updateMouseInput();
}

void InputManager::registerKey(const std::string &name, int glfwKey) {
    keyMap.insert({name, Key(glfwKey)});
}

void InputManager::updateWindowSize(int width, int height) {
    windowWidth = width;
    windowHeight = height;
}

bool InputManager::isKeyPressed(const std::string &name) {
    auto it = keyMap.find(name);
    if (it == keyMap.end()) return false;

    Key &k = it->second;
    if (glfwGetKey(window, k.glfw_key)) {
        if (!k.debounce) {
            k.debounce = true;
            return true;
        }
    } else {
        k.debounce = false;
    }
    return false;
}

bool InputManager::isKeyDown(const std::string &name) {
    auto it = keyMap.find(name);
    if (it == keyMap.end()) return false;

    return glfwGetKey(window, it->second.glfw_key);
}

bool InputManager::isKeyUp(const std::string &name) {
    auto it = keyMap.find(name);
    if (it == keyMap.end()) return true;

    return !glfwGetKey(window, it->second.glfw_key);
}
