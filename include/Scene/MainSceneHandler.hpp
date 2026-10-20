#pragma once

#include "SceneHandler.hpp"

class HUDManager;

class MainSceneHandler : public SceneHandler {
  private:
    static const std::map<std::string_view, Interactable> interMap;

    HUDManager *hudManager;
    GlobalUniformBufferObject gubo{};
    UniformBufferObject ubo{};

    glm::vec4 debugView = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    float elapsedT = 0.0f;
    bool increase = true;
    bool solid = false;

  public:
    MainSceneHandler(SceneLoader *sceneLoader, HUDManager *hudManager);
    ~MainSceneHandler() override = default;

    void updateUniformBuffers(uint32_t currentImage, Player &p, glm::mat4 viewPrjMat, float deltaT,
                              void *data) override;

    void onInteractionReach(std::string_view id, Player &p) override;

    void offInteractionReach(std::string_view id, Player &p) override;

    void onEvent(Player &p, Key k) override;
};
