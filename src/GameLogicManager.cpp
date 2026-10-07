#include "GameLogicManager.hpp"

#include "SceneManager.hpp"

void GameLogicManager::gameLogic(glm::vec3 m, glm::vec3 r, bool fire, float deltaT) {
    cameraController.update(viewPrjMatrix ,cameraPos, m, r, deltaT);
}

void GameLogicManager::onResize(int w, int h) {
    cameraController.onResize(w, h);
}

void GameLogicManager::executeCommand(Command command) {
    switch (command) {
        case Command::UPDATE_TEXTURE:
            debugView.z += 1.0f;
            if (debugView.z > 3.5f) {
                debugView.z = 0.0f;
            }
            break;
        default:
            break;
    }
}

void GameLogicManager::update(uint32_t currentImage, SceneManager *sceneManager, float deltaT) {
    sceneManager->updateUniformBuffer(currentImage, debugView, viewPrjMatrix, cameraPos, deltaT);
}
