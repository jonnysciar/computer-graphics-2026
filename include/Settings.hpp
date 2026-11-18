#pragma once

#include "Listener/Listener.hpp"
#include "InputManager.hpp"
#include <iostream>

class Settings : public Listener<Key> {

  public:
    bool enableShadows = false;
    int cookTorrance = 0;
    int dynamicLight = false;

    Settings() = default;

    void onEvent(Key key) override {
        switch (key.glfw_key) {
        case GLFW_KEY_1:
            enableShadows = !enableShadows;
            break;
        case GLFW_KEY_2:
            cookTorrance = (cookTorrance + 1) % 3;
            std::cout << "Shading is " << cookTorrance << std::endl;
            break;
        case GLFW_KEY_3:
            dynamicLight = !dynamicLight;
            std::cout << "Dynamic light is " << (dynamicLight ? "enabled" : "disabled") << std::endl;
            break;
        default:
            break;
        }
    }
};