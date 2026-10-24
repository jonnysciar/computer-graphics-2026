#pragma once

#include "SceneHandler.hpp"

class Room2Handler : public SceneHandler {
  public:
    Room2Handler(SceneLoader *sceneLoader);
    ~Room2Handler() override = default;

    void updateUniformBuffers(uint32_t currentImage, Player &p, glm::mat4 viewPrjMat, float deltaT,
                              void *data) override;

    void onInteractionReach(std::string_view id, Player &p) override;

    void offInteractionReach(std::string_view id, Player &p) override;

    void onEvent(Player &p, Key k) override;
};
