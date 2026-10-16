// This has been adapted from the Vulkan tutorial

#include "Game.hpp"
#include <SceneHandler.hpp>

// What to do when the window changes size
void Game::onWindowResize(int w, int h) {
    std::cout << "Window resized to: " << w << " x " << h << "\n";
    gameLogicManager->onResize(w, h);
    sceneLoader->onResize(w, h);
    hudManager->onResize(w, h);
}

// Here you load and setup all your Vulkan Models and Texutures.
// Here you also create your Descriptor set layouts and load the shaders for the pipelines
void Game::localInit() {
    sceneLoader = std::make_unique<SceneLoader>(this, "assets/models/scene.json", "shaders/scene.vert.spv",
                                                "shaders/scene.frag.spv", 20, windowWidth, windowHeight,
                                                sizeof(GlobalUniformBufferObject), sizeof(UniformBufferObject));
    hudManager = std::make_unique<HUDManager>(this, windowWidth, windowHeight);
    gameLogicManager = std::make_unique<GameLogicManager>(
        window, sceneLoader.get(), hudManager.get(),
        InputManager(window, static_cast<int>(windowWidth), static_cast<int>(windowHeight)));
}

// Here you create your pipelines and Descriptor Sets!
void Game::pipelinesAndDescriptorSetsInit() {
    sceneLoader->pipelinesAndDescriptorSetsInit();
    hudManager->pipelinesAndDescriptorSetsInit();
}

// Here is where you update the uniforms.
void Game::updateUniformBuffer(uint32_t currentImage) {
    float deltaT;
    auto m = glm::vec3(0.0f);
    auto r = glm::vec3(0.0f);
    bool fire = false;
    getSixAxis(deltaT, m, r, fire);

    gameLogicManager->gameLogic(currentImage, m, r, fire, deltaT);
    hudManager->updateUniformBuffer(currentImage, deltaT);
}

// Here it is the creation of the command buffer:
// You send to the GPU all the objects you want to draw,
// with their buffers and textures

// Here you destroy all the Models, Texture and Desc. Set Layouts you created!
// You also have to destroy the pipelines
void Game::localCleanup() {
    sceneLoader->cleanUp();
    hudManager->cleanUp();
}

// Here you destroy your pipelines and Descriptor Sets!
void Game::pipelinesAndDescriptorSetsCleanup() {
    sceneLoader->pipelinesAndDescriptorSetsCleanup();
    hudManager->pipelinesAndDescriptorSetsCleanup();
}

// Here you set the main application parameters
void Game::setWindowParameters() {
    // window size, titile and initial background
    windowWidth = 800;
    windowHeight = 600;
    windowTitle = "The Hunted Castle";
    windowResizable = GLFW_TRUE;
}
