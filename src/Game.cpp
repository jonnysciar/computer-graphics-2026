// This has been adapted from the Vulkan tutorial

#include "Game.hpp"
#include "Scene/MainSceneHandler.hpp"
#include "Scene/Room1Handler.hpp"
#include "Scene/Room2Handler.hpp"

// What to do when the window changes size
void Game::onWindowResize(int w, int h) {
    std::cout << "Window resized to: " << w << " x " << h << "\n";

    renderPass.width = w;
    renderPass.height = h;

    std::ranges::for_each(sceneLoaders, [w, h](auto &loader) { loader->onResize(w, h); });
    hudManager->onResize(w, h);
    inputManager->onResize(w, h);

    cameraController.onResize(w, h);
}

// Here you load and setup all your Vulkan Models and Texutures.
// Here you also create your Descriptor set layouts and load the shaders for the pipelines
void Game::localInit() {
    hudManager = std::make_unique<HUDManager>(this, windowWidth, windowHeight);

    sceneLoaders.resize(nScenes);
    sceneLoaders[0] = std::make_unique<SceneLoader>(this, "assets/models/general_scene.json", windowWidth, windowHeight,
                                   sizeof(GlobalUniformBufferObject), sizeof(UniformBufferObject));
    sceneLoaders[1] = std::make_unique<SceneLoader>(this, "assets/models/room_1/scene.json", windowWidth, windowHeight,
                                       sizeof(GlobalUniformBufferObject), sizeof(UniformBufferObject));
    sceneLoaders[2] = std::make_unique<SceneLoader>(this, "assets/models/room_2/scene.json", windowWidth, windowHeight,
                                        sizeof(GlobalUniformBufferObject), sizeof(UniformBufferObject));
    sceneLoaders[3] = std::make_unique<SceneLoader>(this, "assets/models/room_3/scene.json", windowWidth, windowHeight,
                                    sizeof(GlobalUniformBufferObject), sizeof(UniformBufferObject));
    sceneLoaders[4] = std::make_unique<SceneLoader>(this, "assets/models/room_4/scene.json", windowWidth, windowHeight,
                                sizeof(GlobalUniformBufferObject), sizeof(UniformBufferObject));
    sceneLoaders[5] = std::make_unique<SceneLoader>(this, "assets/models/room_5/scene.json", windowWidth, windowHeight,
                            sizeof(GlobalUniformBufferObject), sizeof(UniformBufferObject));
    sceneLoaders[6] = std::make_unique<SceneLoader>(this, "assets/models/room_6/scene.json", windowWidth, windowHeight,
                        sizeof(GlobalUniformBufferObject), sizeof(UniformBufferObject));
    sceneLoaders[7] = std::make_unique<SceneLoader>(this, "assets/models/room_7/scene.json", windowWidth, windowHeight,
                    sizeof(GlobalUniformBufferObject), sizeof(UniformBufferObject));

    renderPass.init(this);
    submitCommandBuffer("main", 0, populateCommandBufferAccess, this);

    sceneHandlers.resize(nScenes);
    sceneHandlers[0] = std::make_unique<MainSceneHandler>(sceneLoaders[0].get(), hudManager.get());
    sceneHandlers[1] = std::make_unique<Room1Handler>(sceneLoaders[1].get());
    sceneHandlers[2] = std::make_unique<Room2Handler>(sceneLoaders[2].get());
    sceneHandlers[3] = std::make_unique<Room2Handler>(sceneLoaders[3].get());
    sceneHandlers[4] = std::make_unique<Room2Handler>(sceneLoaders[4].get());
    sceneHandlers[5] = std::make_unique<Room2Handler>(sceneLoaders[5].get());
    sceneHandlers[6] = std::make_unique<Room2Handler>(sceneLoaders[6].get());
    sceneHandlers[7] = std::make_unique<Room2Handler>(sceneLoaders[7].get());

    inputManager = std::make_unique<InputManager>(window, windowWidth, windowHeight);
    inputManager->Listenable<Player &, Key>::addListener(sceneHandlers[0]->Listener<Player &, Key>::getPtr());
    inputManager->Listenable<Player&, Key>::addListener(player.getPtr());
}

// Here you create your pipelines and Descriptor Sets!
void Game::pipelinesAndDescriptorSetsInit() {
    renderPass.create();
    std::ranges::for_each(sceneLoaders, [this](auto &loader) { loader->pipelinesAndDescriptorSetsInit(renderPass); });
    hudManager->pipelinesAndDescriptorSetsInit();
}

// Here is where you update the uniforms.
void Game::updateUniformBuffer(uint32_t currentImage) {float deltaT = handleMovement();

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
    renderPass.destroy();
    hudManager->cleanUp();
}

void Game::pipelinesAndDescriptorSetsCleanup() {
    std::ranges::for_each(sceneLoaders, [](auto &loader) { loader->pipelinesAndDescriptorSetsCleanup(); });
    renderPass.cleanup();
    hudManager->pipelinesAndDescriptorSetsCleanup();
}

void Game::populateCommandBufferAccess(VkCommandBuffer commandBuffer, int currentImage, void *params) {
    auto *game = static_cast<Game *>(params);
    game->populateCommandBuffer(commandBuffer, currentImage);
}

void Game::setWindowParameters() {
    // window size, titile and initial background
    windowWidth = 800;
    windowHeight = 600;
    windowTitle = "The Hunted Castle";
    windowResizable = GLFW_TRUE;
}

void Game::populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) {
    renderPass.begin(commandBuffer, currentImage);
    std::ranges::for_each(sceneLoaders, [commandBuffer, currentImage](auto &loader) {
        loader->populateCommandBuffer(commandBuffer, currentImage);
    });
    renderPass.end(commandBuffer);
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
