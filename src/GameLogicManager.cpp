#include "GameLogicManager.hpp"

#include "SceneManager.hpp"

void GameLogicManager::gameLogic(glm::vec3 m, glm::vec3 r, bool fire, float deltaT) {
    cameraController.update(viewPrjMatrix ,cameraPos, m, r, deltaT, &collisionDetector);
    collisionDetector.update(cameraPos);
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
        case Command::INTERACT:
            interactPressed = true;
            break;
        default:
            break;
    }
}

void GameLogicManager::update(uint32_t currentImage, SceneManager *sceneManager, float deltaT) {
    // The scene is available only here: register the colliders (once)
    // da vedere meglio!!
    if (!collidersRegistered) {
        // objects the player cannot walk through
        for (const char *id : {"sphere", "cube", "statue"}) {
            collisionDetector.addSolid(sceneManager->getCollider(id));
        }
        // objects the player can pick up
        interactionController.add("softball", sceneManager->getCollider("softball"));
        collidersRegistered = true;
    }

    std::string pickedId;
    if (interactionController.update(collisionDetector, interactPressed, pickedId)) {
        sceneManager->hideInstance(pickedId);
    }
    interactPressed = false;

    sceneManager->setPrompt(interactionController.focused().empty() ? "" : "Press E to pick up");

    sceneManager->updateUniformBuffer(currentImage, debugView, viewPrjMatrix, cameraPos, deltaT);
}
