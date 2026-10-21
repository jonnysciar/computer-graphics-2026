#pragma once

#include "Listener/Listener.hpp"
#include "InputManager.hpp"

#include <vulkan/vulkan.h>
#include <modules/Starter.hpp>
#include <modules/Colliders.hpp>

class Player : public Listener<Player &, Key> {
    static constexpr float BODY_RADIUS = 0.4f;
    static constexpr float REACH_RADIUS = 2.0f;

  public:
    glm::vec3 pos = glm::vec3(0.0f, 1.5f, 10.0f);
    Collider bodyCollider;
    Collider reachCollider;

    Player() {
        bodyCollider.initSphere(0.0f, 0.0f, 0.0f, BODY_RADIUS);
        reachCollider.initSphere(0.0f, 0.0f, 0.0f, REACH_RADIUS);
    }

    void updateColliders() {
        glm::mat4 matrix = glm::translate(glm::mat4(1.0f), pos);
        bodyCollider.setWorldMatrix(matrix);
        reachCollider.setWorldMatrix(matrix);
    }

    void onEvent(Player &, Key key) override {
        switch (key.glfw_key) {
        case GLFW_KEY_Q:
            pos.y += 1.0;
            break;
        case GLFW_KEY_Z:
            pos.y -= 1.0;
            break;
        default:
            break;
        }
    }
};