#include "Scene/HandlerHelper.hpp"

#include <modules/Starter.hpp>
#include <modules/Scene.hpp>

namespace HandlerHelper {
    void hideInstance(Instance *instance) {
        instance->Wm = glm::translate(glm::mat4(1), glm::vec3(0.0f, -1000.0f, 0.0f));
        instance->C->setWorldMatrix(instance->Wm);
    }
    bool isTorchPuzzleSatisfied(std::vector<Torch> torches) {
        bool isPuzzleSatisfied = true;
        int counter = 0;
        for (Torch torch : torches) {
            if (torch.id == "flame_2_1" && !torch.on) {
                isPuzzleSatisfied = false;
            }
            if (torch.id == "flame_2_2" &&  torch.on) {
                isPuzzleSatisfied = false;
            }
            if (torch.id == "flame_2_3" && !torch.on) {
                isPuzzleSatisfied = false;
            }
            if (torch.id == "flame_2_4" &&  torch.on) {
                isPuzzleSatisfied = false;
            }
        }
        return isPuzzleSatisfied;
    }
} // namespace HandlerHelper
