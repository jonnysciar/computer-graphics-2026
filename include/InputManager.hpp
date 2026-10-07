#pragma once

#include <GLFW/glfw3.h>

#include <map>
#include <string>

struct MouseState {
    double pixelX, pixelY; // Position in pixels (window coordinates)
    float ndcX, ndcY; // Position in NDC (-1 to +1)

    bool leftJustClicked; // True only on the frame when pressed
    bool leftJustReleased; // True only on the frame when released

    MouseState()
        : pixelX(0.0), pixelY(0.0), ndcX(0.0f), ndcY(0.0f),
          leftJustClicked(false), leftJustReleased(false) {
    }
};

struct Key {
    bool debounce;
    int glfw_key;

    explicit Key(int k) : debounce(false), glfw_key(k) {
    }
};

class InputManager {

private:
    GLFWwindow *window;
    int windowWidth;
    int windowHeight;

    // Previous states (for detecting transitions)
    bool prevMouseLeftPressed;

    // Current states (updated every frame)
    MouseState mouseState;

    std::map<std::string, Key> keyMap;

    // Updates mouse position and button states
    void updateMouseInput();

public:

    InputManager(GLFWwindow *win, int width, int height);

    // To be called once per frame
    void update(float deltaT);

    const MouseState &getMouseState() const { return mouseState; }

    // Associates a name to a GLFW key
    void registerKey(const std::string &name, int glfwKey);

    // Needed for NDC conversion, call it when the window is resized
    void updateWindowSize(int width, int height);

    // True only on the frame in which the key goes from released to pressed
    bool isKeyPressed(const std::string &name);

    // True while the key is held down
    bool isKeyDown(const std::string &name);

    // True while the key is not held down (also for unregistered names)
    bool isKeyUp(const std::string &name);
};
