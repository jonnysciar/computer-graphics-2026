#pragma once

#include "SceneHandler.hpp"

class HUDManager;

class MainSceneHandler : public SceneHandler {
  private:
    HUDManager *hudManager;

    glm::vec4 debugView = glm::vec4(5.0f, 1.0f, 1.0f, 1.0f);
    float elapsedT = 0.0f;
    bool increase = true;
    bool solid = false;

  public:
    MainSceneHandler(SceneLoader *sceneLoader, HUDManager *hudManager);
    ~MainSceneHandler() override = default;

    void onInteractionReach(std::string_view id, Player &p) override;

    void offInteractionReach(std::string_view id, Player &p) override;

    void onEvent(Player &p, Key k) override;
};
