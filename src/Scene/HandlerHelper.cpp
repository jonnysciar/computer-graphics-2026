#include "Scene/HandlerHelper.hpp"

#include <modules/Starter.hpp>
#include <modules/Scene.hpp>

namespace HandlerHelper {

void hideInstance(Instance *instance) {
    instance->Wm = glm::translate(glm::mat4(1), glm::vec3(0.0f, -1000.0f, 0.0f));
    instance->C->setWorldMatrix(instance->Wm);
}

} // namespace HandlerHelper
