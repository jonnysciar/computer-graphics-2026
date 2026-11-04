#pragma once

#include "InputManager.hpp"
#include "glm/gtx/euler_angles.hpp"

#include <vulkan/vulkan.h>
#include <modules/Starter.hpp>
#include <modules/Scene.hpp>

#include <string>
#include <unordered_set>

class  Door {
    private:
        const std::string id;
        bool isLocked;
        bool isOpen;

    public:
        Door(const char * str, bool islocked, bool isopen) {};

        bool getIsLocked() {
            return isLocked;
        }
        void setIsLocked (bool islocked) {
            isLocked = islocked;
        }
        bool getIsOpen() {
            return isOpen;
        }
        void setIsOpen(bool isopen) {
            isOpen = isopen;
        }
        void rotateInstanceY(Instance* instance, float angle) {
            if (instance == nullptr || instance->C == nullptr) {
                return;
            }
            glm::vec3 pivot = glm::vec3(instance->Wm[3]);

            glm::mat4 ti = glm::translate(glm::mat4(1.0f), -pivot);
            glm::mat4 ry = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
            glm::mat4 tf = glm::translate(glm::mat4(1.0f), pivot);

            instance->Wm = tf * ry * ti * instance->Wm;

            instance->C->setWorldMatrix(instance->Wm);
        }
};