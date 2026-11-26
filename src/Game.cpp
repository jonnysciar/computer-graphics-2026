// This has been adapted from the Vulkan tutorial

#include "Game.hpp"

#include "CameraController.hpp"
#include "uniform_buffers.hpp"
#include "Scene/Room1Handler.hpp"
#include "Scene/Room2Handler.hpp"
#include "Scene/Room3Handler.hpp"
#include "Scene/Room4Handler.hpp"
#include "Scene/Room6Handler.hpp"
#include "Scene/Room7Handler.hpp"

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
    sceneLoader = std::make_unique<SceneLoader>(this, "assets/models/scene.json", device, physicalDevice, windowWidth,
                                                windowHeight);

    submitCommandBuffer("main", 0, populateCommandBufferAccess, this);

    gameStatus.loadLights("assets/models/lights.json");

    if (gameStatus.torches.size() != NUM_LIGHTS) {
        std::cout << "Number of torch lights: " << gameStatus.torches.size() << " mismatches " << NUM_LIGHTS << "\n";
        // std::exit(1);
    }

    sceneHandlers.push_back(std::make_unique<Room1Handler>(sceneLoader.get(), hudManager.get(), gameStatus));
    sceneHandlers.push_back(std::make_unique<Room2Handler>(sceneLoader.get(), hudManager.get(), gameStatus));
    sceneHandlers.push_back(std::make_unique<Room3Handler>(sceneLoader.get(), hudManager.get()));
    sceneHandlers.push_back(std::make_unique<Room4Handler>(sceneLoader.get(), hudManager.get()));
    sceneHandlers.push_back(std::make_unique<Room6Handler>(sceneLoader.get(), hudManager.get(), gameStatus));
    sceneHandlers.push_back(std::make_unique<Room7Handler>(sceneLoader.get(), hudManager.get(), gameStatus));

    inputManager = std::make_unique<InputManager>(window, windowWidth, windowHeight);
    inputManager->Listenable<Key>::addListener(gameStatus.player.getPtr());
    inputManager->Listenable<Key>::addListener(gameStatus.settings.getPtr());
    std::ranges::for_each(sceneHandlers, [&](auto &handler) {
        inputManager->Listenable<GameStatus &, Key>::addListener(
            handler->template Listener<GameStatus &, Key>::getPtr());
    });
}

// Here you create your pipelines and Descriptor Sets!
void Game::pipelinesAndDescriptorSetsInit() {
    sceneLoader->pipelinesAndDescriptorSetsInit();
    hudManager->pipelinesAndDescriptorSetsInit();

    // the shadow casters' descriptor sets are recreated: their uniforms must be written again
    gameStatus.shadowsDirty = true;
}

// Here is where you update the uniforms.
void Game::updateUniformBuffer(uint32_t currentImage) {
    float deltaT = handleMovement();

    inputManager->update(gameStatus, deltaT);

    std::ranges::for_each(sceneHandlers, [&](auto &handler) { handler->checkInteractionReach(gameStatus); });

    sceneLoader->updateUniformBuffer(currentImage, gameStatus, cameraController);
    updateShadows(currentImage);
    hudManager->updateUniformBuffer(currentImage, deltaT);
}

// The shadow maps are kept between frames: they are rendered again only in the frame after the geometry changed
void Game::updateShadows(uint32_t currentImage) {
    // e.g. a door opened or a key picked up; switching a torch on or off does not change the shadow maps
    if (sceneLoader->geometryChanged())
        gameStatus.shadowsDirty = true;

    if (gameStatus.shadowsDirty) {
        gameStatus.shadowsDirty = false;
        shadowPassQueued = true;
        sceneLoader->prepareShadowPass(currentImage, gameStatus);
        submitCommandBuffer("shadows", -1, populateShadowCommandBufferAccess, this);
    } else if (shadowPassQueued) {
        // re-submitted empty: the Starter's removeBuffer() would leave a null buffer that drawFrame dereferences
        shadowPassQueued = false;
        submitCommandBuffer("shadows", -1, populateShadowCommandBufferAccess, this);
    }
}

void Game::localCleanup() {
    sceneLoader->cleanUp();
    hudManager->cleanUp();
}

void Game::pipelinesAndDescriptorSetsCleanup() {
    sceneLoader->pipelinesAndDescriptorSetsCleanup();
    hudManager->pipelinesAndDescriptorSetsCleanup();
}

void Game::populateCommandBufferAccess(VkCommandBuffer commandBuffer, int currentImage, void *params) {
    auto *game = static_cast<Game *>(params);
    game->populateCommandBuffer(commandBuffer, currentImage);
}

void Game::populateShadowCommandBufferAccess(VkCommandBuffer commandBuffer, int currentImage, void *params) {
    auto *game = static_cast<Game *>(params);
    if (game->shadowPassQueued) {
        game->sceneLoader->populateShadowCommandBuffer(commandBuffer, currentImage);
    }
}

void Game::setWindowParameters() {
    // window size, titile and initial background
    windowWidth = 800;
    windowHeight = 600;
    windowTitle = "The Hunted Castle";
    windowResizable = GLFW_TRUE;
}

void Game::populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) {
    sceneLoader->populateCommandBuffer(commandBuffer, currentImage);
}

float Game::handleMovement() {
    float deltaT;
    auto m = glm::vec3(0.0f);
    auto r = glm::vec3(0.0f);
    bool fire = false;
    getSixAxis(deltaT, m, r, fire);

    glm::vec3 newPos = cameraController.computePos(gameStatus.player.pos, m, r, deltaT);
    gameStatus.player.pos = sceneLoader->getCollisionDetector()->resolveCollision(gameStatus.player, newPos);
    cameraController.updateViewPrjMat(gameStatus.player.pos);
    gameStatus.player.updateColliders();

    return deltaT;
}
