#pragma once

#include <modules/Scene.hpp>

#include "SceneHandler.hpp"
#include "structs.hpp"

class MainSceneHandler : public SceneHandler {
public:
    struct params {
        glm::mat4 viewPrjMat;
        glm::vec3 cameraPos;
        glm::vec4 debugView;
    };

    explicit MainSceneHandler(std::vector<Instance *> instances) : SceneHandler(std::move(instances)) {}
    ~MainSceneHandler() override = default;
    void updateUniformBuffers(uint32_t currentImage, void *data) override {
        auto *params = static_cast<MainSceneHandler::params *>(data);
        // defines the global parameters for the uniform
        GlobalUniformBufferObject gubo{};
        gubo.lightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f) * 5.0f;
        gubo.pointParams = glm::vec3(params->debugView.x, 2.0, 0.0);
        gubo.eyePos = params->cameraPos;
        gubo.ambientUpper = glm::vec4(0.05f, 0.05f, 1.0f, 0.0f); // sky blue
        gubo.ambientLower = glm::vec4(0.0f, 0.05f, 1.0f, 0.0f); // warm ground
        gubo.ambientDir = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f); // world up
        gubo.debugView = params->debugView;

        // Each instance is drawn with its own world matrix (from scene.json), the same one used by its collider
        UniformBufferObject ubo{};
        for (auto *instance: instances) {

            ubo.mMat = instance->Wm;
            ubo.mvpMat = params->viewPrjMat * ubo.mMat;
            ubo.nMat = glm::inverse(glm::transpose(ubo.mMat));
            instance->DS[0][0]->map(currentImage, &gubo, 0);
            instance->DS[0][1]->map(currentImage, &ubo, 0);
        }
    }
};
