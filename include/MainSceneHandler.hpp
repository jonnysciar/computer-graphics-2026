#pragma once

#include <modules/Scene.hpp>

#include "InputManager.hpp"
#include "SceneHandler.hpp"
#include "structs.hpp"

inline void hideInstance(Instance *instance) {
    instance->Wm = glm::translate(glm::mat4(1), glm::vec3(0.0f, -1000.0f, 0.0f));
    instance->C->setWorldMatrix(instance->Wm);
}

class MainSceneHandler : public SceneHandler {
  private:
    static std::map<std::string_view, Interactable> interMap;

    HUDManager *hudManager;
    GlobalUniformBufferObject gubo{};
    UniformBufferObject ubo{};

    glm::vec4 debugView = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    float elapsedT = 0.0f;
    bool increase = true;

  public:
    explicit MainSceneHandler(SceneLoader *sceneLoader, HUDManager *hudManager)
        : SceneHandler(sceneLoader), hudManager(hudManager) {

        std::vector<Interactable> interactables;
        interactables.reserve(interMap.size());
        for (const auto &instance : instances) {
            auto it = interMap.find(*instance->id);
            if (it == interMap.end()) continue;

            auto inter = it->second;
            if (!inter.solid)
                collisionDetector->removeCollider(instance->C);

            inter.instance = instance;
            interactables.push_back(std::move(inter));
        }

        interactionController =
            std::make_unique<InteractionController>(collisionDetector.get(), std::move(interactables));
    }
    ~MainSceneHandler() override = default;
    void updateUniformBuffers(uint32_t currentImage, Player &p, glm::mat4 viewPrjMat, float deltaT,
                              void *data) override {
        elapsedT += deltaT;
        if (elapsedT > 0.01f) {
            if (increase)
                debugView.x += 0.01f;
            else
                debugView.x -= 0.01f;

            if (debugView.x < 0.5f || debugView.x > 2.0f) increase = !increase;
            elapsedT = 0.0f;
        }

        debugView.y = deltaT;

        // defines the global parameters for the uniform
        gubo.lightColor = glm::vec4(1.0f, 0.5f, 0.5f, 1.0f);
        gubo.pointParams = glm::vec3(debugView.x, 2.0, 0.0);
        gubo.eyePos = p.pos;
        gubo.ambientUpper = glm::vec4(0.05f, 0.05f, 1.0f, 0.0f); // sky blue
        gubo.ambientLower = glm::vec4(0.0f, 0.05f, 1.0f, 0.0f);  // warm ground
        gubo.ambientDir = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);     // world up
        gubo.debugView = debugView;

        // Each instance is drawn with its own world matrix (from scene.json), the same one used by its collider
        for (auto *instance : instances) {
            ubo.mMat = instance->Wm;
            ubo.mvpMat = viewPrjMat * ubo.mMat;
            ubo.nMat = glm::inverse(glm::transpose(ubo.mMat));
            instance->DS[0][0]->map(currentImage, &ubo, 0);
            instance->DS[0][0]->map(currentImage, &gubo, 1);
        }
    }

    void onInteractionReach(std::string_view id, Player &p) override {
        if (!interMap.contains(id)) return;

        hudManager->setPrompt("Press E to pick up");
    }

    void offInteractionReach(std::string_view id, Player &p) override {
        if (!interMap.contains(id)) return;

        hudManager->setPrompt("");
    }

    void onEvent(Player &p, Key k) override {
        switch (k.glfw_key) {
        case GLFW_KEY_E:
            interactionController->interact(p);
            sceneLoader->refreshColliders();
            break;
        case GLFW_KEY_L:
            debugView.x = 0.5f;
            increase = true;
        default:
            break;
        }
    }
};

std::map<std::string_view, Interactable> MainSceneHandler::interMap = {
    {"softball", Interactable{.instance = nullptr,
                              .solid = false,
                              .callback = [](Interactable &i, Player &p) { hideInstance(i.instance); }}},
};
