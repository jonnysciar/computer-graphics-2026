// This has been adapted from the Vulkan tutorial

#include "Game.hpp"
#include "CameraController.hpp"

// What to do when the window changes size
void Game::onWindowResize(int w, int h) {
    std::cout << "Window resized to: " << w << " x " << h << "\n";
    gameLogicManager->onResize(w, h);
    sceneManager->onResize(w, h);
    inputManager->updateWindowSize(w, h);
}

// Here you load and setup all your Vulkan Models and Texutures.
// Here you also create your Descriptor set layouts and load the shaders for the pipelines
void Game::localInit() {
    inputManager = std::make_unique<InputManager>(window, windowWidth, windowHeight);
    sceneManager = std::make_unique<SceneManager>(this, "", windowWidth, windowHeight);
    gameLogicManager = std::make_unique<GameLogicManager>();

    inputManager->registerKey("change_texture", GLFW_KEY_3);
    inputManager->registerKey("quit", GLFW_KEY_ESCAPE);
}

// Here you create your pipelines and Descriptor Sets!
void Game::pipelinesAndDescriptorSetsInit() {
    sceneManager->pipelinesAndDescriptorSetsInit();
}

float Game::GameLogic(){
    float deltaT;
    auto m = glm::vec3(0.0f);
    auto r = glm::vec3(0.0f);
    bool fire = false;
    getSixAxis(deltaT, m, r, fire);

    gameLogicManager->gameLogic(m, r, fire, deltaT);

    return deltaT;
}

// Here is where you update the uniforms.
void Game::updateUniformBuffer(uint32_t currentImage) {
    // press 3 to change the texture
    if (inputManager->isKeyPressed("change_texture")) {
        gameLogicManager->executeCommand(GameLogicManager::Command::UPDATE_TEXTURE);
    }
    if (inputManager->isKeyDown("quit")) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    float deltaT = GameLogic();
    inputManager->update(deltaT);

    gameLogicManager->update(currentImage, sceneManager.get(), deltaT);
}

// Here it is the creation of the command buffer:
// You send to the GPU all the objects you want to draw,
// with their buffers and textures

// Here you destroy all the Models, Texture and Desc. Set Layouts you created!
// You also have to destroy the pipelines
void Game::localCleanup() {
    sceneManager->cleanUp();
}

// Here you destroy your pipelines and Descriptor Sets!
void Game::pipelinesAndDescriptorSetsCleanup() {
    sceneManager->pipelinesAndDescriptorSetsCleanup();
}

// Here you set the main application parameters
void Game::setWindowParameters() {
    // window size, titile and initial background
    windowWidth = 800;
    windowHeight = 600;
    windowTitle = "The Hunted Castle";
    windowResizable = GLFW_TRUE;
}
