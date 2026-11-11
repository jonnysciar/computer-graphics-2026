#pragma once

#include "Listener/Listener.hpp"
#include "InputManager.hpp"

class Settings : public Listener<Key> {

  public:
    bool enableShadows = false;

    Settings() = default;

    void onEvent(Key key) override {
        switch (key.glfw_key) {
        case GLFW_KEY_1:
            enableShadows = !enableShadows;
            break;
        default:
            break;
        }
    }
};