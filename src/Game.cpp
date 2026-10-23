// This has been adapted from the Vulkan tutorial

#include "Game.hpp"
#include "Scene/MainSceneHandler.hpp"

// What to do when the window changes size
void Game::onWindowResize(int w, int h) {
    std::cout << "Window resized to: " << w << " x " << h << "\n";
    std::ranges::for_each(sceneLoaders, [w, h](auto &loader) { loader->onResize(w, h); });
    hudManager->onResize(w, h);
    inputManager->onResize(w, h);

    cameraController.onResize(w, h);
}

// Here you load and setup all your Vulkan Models and Texutures.
// Here you also create your Descriptor set layouts and load the shaders for the pipelines
void Game::localInit() {
    hudManager = std::make_unique<HUDManager>(this, windowWidth, windowHeight);

    sceneLoaders.resize(2);
    sceneLoaders[0] = std::make_unique<SceneLoader>(this, "assets/models/general_scene.json", windowWidth, windowHeight,
                                        sizeof(GlobalUniformBufferObject), sizeof(UniformBufferObject), true);
    sceneLoaders[1] = std::make_unique<SceneLoader>(this, "assets/models/room_1/scene.json", windowWidth, windowHeight,
                                       sizeof(GlobalUniformBufferObject), sizeof(UniformBufferObject));

    submitCommandBuffer(
        "main", 0,
        [](VkCommandBuffer commandBuffer, int currentImage, void *params) {
            auto *loaders = static_cast<std::vector<std::unique_ptr<SceneLoader>> *>(params);
            for (auto &loader : *loaders) {
                loader->populateCommandBuffer(commandBuffer, currentImage);
            }
        },
        &sceneLoaders);

    sceneHandlers.resize(2);
    sceneHandlers[0] = std::make_unique<MainSceneHandler>(sceneLoaders[0].get(), hudManager.get());
    sceneHandlers[1] = std::make_unique<MainSceneHandler>(sceneLoaders[1].get(), hudManager.get());

    inputManager = std::make_unique<InputManager>(window, windowWidth, windowHeight);
    inputManager->Listenable<Player &, Key>::addListener(sceneHandlers[0]->Listener<Player &, Key>::getPtr());
    inputManager->Listenable<Player&, Key>::addListener(player.getPtr());
}

// Here you create your pipelines and Descriptor Sets!
void Game::pipelinesAndDescriptorSetsInit() {
    std::ranges::for_each(sceneLoaders, [](auto &loader) { loader->pipelinesAndDescriptorSetsInit(); });
    hudManager->pipelinesAndDescriptorSetsInit();
}

// Here is where you update the uniforms.
void Game::updateUniformBuffer(uint32_t currentImage) {
    float deltaT = handleMovement();

    inputManager->update(player, deltaT);

    std::ranges::for_each(sceneHandlers, [&](auto &handler) {
        handler->checkInteractionReach(player);
        handler->updateUniformBuffers(currentImage, player, viewPrjMat, deltaT, nullptr);
    });
    std::ranges::for_each(sceneLoaders, [this, currentImage](auto &loader) { loader->updateUniformBuffer(currentImage, viewPrjMat);; });
    hudManager->updateUniformBuffer(currentImage, deltaT);
}

void Game::localCleanup() {
    std::ranges::for_each(sceneLoaders, [](auto &loader) { loader->cleanUp();});
    hudManager->cleanUp();
}

void Game::pipelinesAndDescriptorSetsCleanup() {
    std::ranges::for_each(sceneLoaders, [](auto &loader) { loader->pipelinesAndDescriptorSetsCleanup(); });
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
    std::ranges::for_each(sceneHandlers, [&](auto &handler) { newPos = handler->resolveCollision(player, newPos); });
    player.pos = newPos;
    viewPrjMat = cameraController.computeViewPrjMat(player.pos);
    player.updateColliders();

    return deltaT;
}
