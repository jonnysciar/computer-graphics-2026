#pragma once

#include "Listener/Listenable.hpp"

#include <GLFW/glfw3.h>

struct GameStatus;
;

struct MouseState {
    double pixelX, pixelY; // Position in pixels (window coordinates)
    float ndcX, ndcY;      // Position in NDC (-1 to +1)

    bool leftJustClicked;  // True only on the frame when pressed
    bool leftJustReleased; // True only on the frame when released

    MouseState() : pixelX(0.0), pixelY(0.0), ndcX(0.0f), ndcY(0.0f), leftJustClicked(false), leftJustReleased(false) {}
};

struct Key {
    int glfw_key;
    bool isDown;

    explicit Key(int k) : glfw_key(k), isDown(false) {}
};

class InputManager : public Listenable<Key>,
                     public Listenable<GameStatus &, Key>,
                     public Listenable<GameStatus &, MouseState> {
  private:
    GLFWwindow *window;
    int windowWidth;
    int windowHeight;

    // Previous states (for detecting transitions)
    bool prevMouseLeftPressed;

    // Current states (updated every frame)
    MouseState mouseState;

    std::vector<Key> keys = {Key{GLFW_KEY_E}, Key{GLFW_KEY_M}, Key{GLFW_KEY_Q}, Key{GLFW_KEY_Z}, Key{GLFW_KEY_L}};

    // Updates mouse position and button states
    void updateMouseInput(GameStatus &p);

    // True only on the frame in which the key goes from released to pressed
    void updatePressedKeys(GameStatus &p);

  public:
    InputManager(GLFWwindow *win, int width, int height);

    // To be called once per frame
    void update(GameStatus &p, float deltaT);

    const MouseState &getMouseState() const { return mouseState; }

    // Needed for NDC conversion, call it when the window is resized
    void onResize(int width, int height);
};
