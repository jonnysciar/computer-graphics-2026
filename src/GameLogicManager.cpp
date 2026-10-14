#include "GameLogicManager.hpp"

#include <utility>

#include "HUDManager.hpp"
#include "SceneManager.hpp"

GameLogicManager::GameLogicManager(GLFWwindow *window, SceneManager *sceneManager, HUDManager *hudManager,
                                   InputManager inputManager) : window(window), sceneManager(sceneManager),
                                                                hudManager(hudManager),
                                                                inputManager(std::move(inputManager)) {
    this->inputManager.registerKey("change_texture", GLFW_KEY_3);
    this->inputManager.registerKey("quit", GLFW_KEY_ESCAPE);
    this->inputManager.registerKey("interact", GLFW_KEY_E);
    this->inputManager.registerKey("increase_light", GLFW_KEY_L);
}

void GameLogicManager::gameLogic(uint32_t currentImage, glm::vec3 m, glm::vec3 r, bool fire, float deltaT) {
    if (inputManager.isKeyPressed("change_texture")) {
        executeCommand(Command::UPDATE_TEXTURE);
    }
    // press E to interact with the objects in reach
    if (inputManager.isKeyPressed("interact")) {
        executeCommand(Command::INTERACT);
    }
    if (inputManager.isKeyDown("quit")) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
    if (inputManager.isKeyPressed("increase_light")) {
        executeCommand(Command::INCREASE_LIGHT);
    }

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

    hudManager->setPrompt(interactionController.focused().empty() ? "" : "Press E to pick up");

    cameraController.update(viewPrjMatrix ,cameraPos, m, r, deltaT, &collisionDetector);
    collisionDetector.update(cameraPos);
    inputManager.update(deltaT);
    sceneManager->updateUniformBuffer(currentImage, debugView, viewPrjMatrix, cameraPos, deltaT);
}

void GameLogicManager::onResize(int w, int h) {
    cameraController.onResize(w, h);
    inputManager.onResize(w, h);
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
        case Command::INCREASE_LIGHT:
            debugView.x += 1.0f;
            if (debugView.x > 10.0f) {
                debugView.x = 0.0f;
            }
        default:
            break;
    }
}
