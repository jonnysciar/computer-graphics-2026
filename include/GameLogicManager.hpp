#pragma once

#include "CameraController.hpp"
#include "CollisionDetector.hpp"
#include "InteractionController.hpp"
#include "InputManager.hpp"

#include <glm/glm.hpp>

class HUDManager;
class SceneManager;

class GameLogicManager {
private:
    GLFWwindow *window;
    CameraController cameraController;
    CollisionDetector collisionDetector;
    InteractionController interactionController;
    SceneManager *sceneManager;
    HUDManager *hudManager;
    InputManager inputManager;

    glm::mat4 viewPrjMatrix = glm::mat4(0.0f);
    glm::vec3 cameraPos = glm::vec3(0.0f, 1.5f, 10.0f);
    glm::vec4 debugView = glm::vec4(1.0f, glm::vec3(0.0f));

    bool collidersRegistered = false;
    bool interactPressed = false;

public:
    enum class Command {
        UPDATE_TEXTURE,
        INTERACT,
        INCREASE_LIGHT,
    };

    GameLogicManager(GLFWwindow *window, SceneManager *sceneManager, HUDManager *hudManager, InputManager inputManager);

    void gameLogic(uint32_t currentImage, glm::vec3 m, glm::vec3 r, bool fire, float deltaT);
    void onResize(int w, int h);
    void executeCommand(Command command);
};
