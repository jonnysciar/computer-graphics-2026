#pragma once

#include "Listener/Listener.hpp"
#include "InputManager.hpp"

#include <vulkan/vulkan.h>
#include <modules/Starter.hpp>
#include <modules/Colliders.hpp>

#include <string>
#include <unordered_set>

class Player : public Listener<Key> {
    static constexpr float BODY_RADIUS = 0.4f;
    static constexpr float REACH_RADIUS = 3.0f;

  public:
    glm::vec3 pos = glm::vec3(0.0f, 3.5f, 10.0f);
    Collider bodyCollider;
    Collider reachCollider;
    std::unordered_set<std::string> collectedItems;
    bool isShadow = false;

    Player() {
        bodyCollider.initSphere(0.0f, 0.0f, 0.0f, BODY_RADIUS);
        reachCollider.initSphere(0.0f, 0.0f, 0.0f, REACH_RADIUS);
    }

    void updateColliders() {
        glm::mat4 matrix = glm::translate(glm::mat4(1.0f), pos);
        bodyCollider.setWorldMatrix(matrix);
        reachCollider.setWorldMatrix(matrix);
    }

    void pickUp(std::string_view id) { collectedItems.emplace(id); }

    bool hasItem(const std::string &id) const { return collectedItems.contains(id); }

    int countItems() const { return static_cast<int>(collectedItems.size()); }

    void onEvent(Key key) override {
        switch (key.glfw_key) {
        case GLFW_KEY_Q:
            pos.y += 1.0;
            break;
        case GLFW_KEY_Z:
            pos.y -= 1.0;
            break;
        case GLFW_KEY_1:
            isShadow = !isShadow;
        default:
            break;
        }
    }
};