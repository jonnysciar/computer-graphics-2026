#pragma once

#include <vulkan/vulkan.h>
#include <modules/Starter.hpp>
#include <modules/Scene.hpp>

#include <string>

class Door {
  private:
    const std::string id;
    bool isLocked;
    bool isOpen;

  public:
    Door(const char *str, bool islocked, bool isopen) : id(str), isLocked(islocked), isOpen(isopen) {};

    bool getIsLocked() { return isLocked; }
    void setIsLocked(bool islocked) { isLocked = islocked; }
    bool getIsOpen() { return isOpen; }
    void setIsOpen(bool isopen) { isOpen = isopen; }

    // room on the other side of the door, -1 if the door does not touch room (its id is "door_<a>_<b>")
    int otherRoom(int room) const {
        const int r1 = id[5] - '0';
        const int r2 = id[7] - '0';
        const int other = r1 == room ? r2 : r2 == room ? r1 : -1;
        return other >= 1 && other <= 9 ? other : -1; // "o" = outside
    }

    void rotateInstanceY(Instance *instance, float angle) {
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
