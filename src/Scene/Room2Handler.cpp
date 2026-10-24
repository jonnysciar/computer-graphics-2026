#include "Scene/Room2Handler.hpp"

Room2Handler::Room2Handler(SceneLoader *sceneLoader) : SceneHandler(sceneLoader) {
    interactionController = std::make_unique<InteractionController>(collisionDetector, std::vector<Interactable>{});
}

void Room2Handler::updateUniformBuffers(uint32_t currentImage, Player &p, glm::mat4 viewPrjMat, float deltaT,
                                        void *data) {
    UniformBufferObject ubo{};
    GlobalUniformBufferObject gubo{};

    // defines the global parameters for the uniform
    gubo.lightColor = glm::vec4(1.0f, 0.5f, 0.5f, 1.0f);
    gubo.pointParams = glm::vec3(5.0, 2.0, 0.0);
    gubo.eyePos = p.pos;
    gubo.ambientUpper = glm::vec4(0.05f, 0.05f, 1.0f, 0.0f); // sky blue
    gubo.ambientLower = glm::vec4(0.0f, 0.05f, 1.0f, 0.0f);  // warm ground
    gubo.ambientDir = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);     // world up

    // Each instance is drawn with its own world matrix (from general_scene.json), the same one used by its collider
    for (auto *instance : instances) {
        ubo.mMat = instance->Wm;
        ubo.mvpMat = viewPrjMat * ubo.mMat;
        ubo.nMat = glm::inverse(glm::transpose(ubo.mMat));
        instance->DS[0][0]->map(currentImage, &ubo, 0);
        instance->DS[0][0]->map(currentImage, &gubo, 1);
    }
}

void Room2Handler::onInteractionReach(std::string_view id, Player &p) {}

void Room2Handler::offInteractionReach(std::string_view id, Player &p) {}

void Room2Handler::onEvent(Player &p, Key k) {
    switch (k.glfw_key) {
    default:
        break;
    }
}