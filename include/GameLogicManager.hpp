#pragma once

#include "CameraController.hpp"

#include <glm/glm.hpp>

class SceneManager;

class GameLogicManager {
private:
    CameraController cameraController;

    glm::mat4 viewPrjMatrix;
    glm::vec3 cameraPos = glm::vec3(0.0f, 1.5f, 10.0f);
    glm::vec4 debugView = glm::vec4(0.0);

public:
    enum class Command {
        UPDATE_TEXTURE,
    };

    void gameLogic(glm::vec3 m, glm::vec3 r, bool fire, float deltaT);
    void onResize(int w, int h);
    void executeCommand(Command command);
    void update(uint32_t currentImage, SceneManager *sceneManager, float deltaT);
};
