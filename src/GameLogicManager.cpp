#include "GameLogicManager.hpp"

#include <utility>

#include "HUDManager.hpp"
#include "SceneLoader.hpp"
#include "MainSceneHandler.hpp"

GameLogicManager::GameLogicManager(GLFWwindow *window, SceneLoader *sceneLoader, HUDManager *hudManager,
                                   InputManager inputManager)
    : window(window), sceneLoader(sceneLoader), hudManager(hudManager), inputManager(std::move(inputManager)) {
    this->inputManager.registerKey("change_texture", GLFW_KEY_3);
    this->inputManager.registerKey("quit", GLFW_KEY_ESCAPE);
    this->inputManager.registerKey("interact", GLFW_KEY_E);
    this->inputManager.registerKey("increase_light", GLFW_KEY_L);

    auto instances = this->sceneLoader->getInstances();
    sceneHandler = std::make_unique<MainSceneHandler>(instances);

    auto c = instances | std::views::transform([](auto *inst) { return inst->C; });
    collisionDetector = std::make_unique<CollisionDetector>(std::vector<Collider *>{c.begin(), c.end()});

    auto c_i = std::ranges::find_if(instances, [](auto *inst) { return *inst->id == "softball"; });
    if (c_i != instances.end()) {
        interactionController.add("softball", (*c_i)->C);
    }
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

    std::string pickedId;
    if (interactionController.update(*collisionDetector, interactPressed, pickedId)) {
        hideInstance(pickedId);
    }
    interactPressed = false;

    hudManager->setPrompt(interactionController.focused().empty() ? "" : "Press E to pick up");

    cameraController.update(viewPrjMatrix ,cameraPos, m, r, deltaT, collisionDetector.get());
    collisionDetector->update(cameraPos);
    inputManager.update(deltaT);

    MainSceneHandler::params p {viewPrjMatrix, cameraPos, debugView};
    sceneHandler->updateUniformBuffers(currentImage, &p);

    sceneLoader->updateUniformBuffer(currentImage, viewPrjMatrix);
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

void GameLogicManager::hideInstance(std::string_view id) {
    auto instances = sceneLoader->getInstances();
    auto it = std::ranges::find_if(instances, [id](auto *inst) { return *inst->id == id; });
    if (it != instances.end()) {
        (*it)->Wm = glm::translate(glm::mat4(1), glm::vec3(0.0f, -1000.0f, 0.0f));
        (*it)->C->setWorldMatrix((*it)->Wm);
    }

    sceneLoader->refreshColliders();
}
