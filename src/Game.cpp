// This has been adapted from the Vulkan tutorial

#include "Game.hpp"
#include "MainSceneHandler.hpp"

// What to do when the window changes size
void Game::onWindowResize(int w, int h) {
    std::cout << "Window resized to: " << w << " x " << h << "\n";
    sceneLoader->onResize(w, h);
    hudManager->onResize(w, h);
    inputManager->onResize(w, h);

    cameraController.onResize(w, h);
}

// Here you load and setup all your Vulkan Models and Texutures.
// Here you also create your Descriptor set layouts and load the shaders for the pipelines
void Game::localInit() {
    hudManager = std::make_unique<HUDManager>(this, windowWidth, windowHeight);

    sceneLoader = std::make_unique<SceneLoader>(this, "assets/models/scene.json", windowWidth, windowHeight,
                                                sizeof(GlobalUniformBufferObject), sizeof(UniformBufferObject));

    sceneHandler = std::make_unique<MainSceneHandler>(sceneLoader.get(), hudManager.get());

    inputManager = std::make_unique<InputManager>(window, windowWidth, windowHeight);
    inputManager->Listenable<Key>::addListener(sceneHandler.get());
}

// Here you create your pipelines and Descriptor Sets!
void Game::pipelinesAndDescriptorSetsInit() {
    sceneLoader->pipelinesAndDescriptorSetsInit();
    hudManager->pipelinesAndDescriptorSetsInit();
}

// Here is where you update the uniforms.
void Game::updateUniformBuffer(uint32_t currentImage) {
    float deltaT = handleMovement();

    inputManager->update(player, deltaT);

    sceneHandler->checkInteractionReach(player);

    sceneHandler->updateUniformBuffers(currentImage, player, viewPrjMat, deltaT, nullptr);
    sceneLoader->updateUniformBuffer(currentImage, viewPrjMat);
    hudManager->updateUniformBuffer(currentImage, deltaT);
}

void Game::localCleanup() {
    sceneLoader->cleanUp();
    hudManager->cleanUp();
}

void Game::pipelinesAndDescriptorSetsCleanup() {
    sceneLoader->pipelinesAndDescriptorSetsCleanup();
    hudManager->pipelinesAndDescriptorSetsCleanup();
}

void Game::setWindowParameters() {
    // window size, titile and initial background
    windowWidth = 800;
    windowHeight = 600;
    windowTitle = "The Hunted Castle";
    windowResizable = GLFW_TRUE;
}

float Game::handleMovement() {
    float deltaT;
    auto m = glm::vec3(0.0f);
    auto r = glm::vec3(0.0f);
    bool fire = false;
    getSixAxis(deltaT, m, r, fire);

    glm::vec3 newPos = cameraController.computePos(player.pos, m, r, deltaT);
    player.pos = sceneHandler->resolveCollision(player, newPos);
    viewPrjMat = cameraController.computeViewPrjMat(player.pos);
    player.updateColliders();

    return deltaT;
}
