// This has been adapted from the Vulkan tutorial

#include "Game.hpp"

#include "CameraController.hpp"
#include "HUDManagerNew.hpp"
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
    hudManagerNew = std::make_unique<HUDManagerNew>(this, windowWidth, windowHeight);
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
    hudManagerNew->pipelinesAndDescriptorSetsInit();
}

// Here is where you update the uniforms.
void Game::updateUniformBuffer(uint32_t currentImage) {
    if (gameStatus.settings.populate) {
        submitCommandBuffer("main", 0, populateCommandBufferAccess, this);
        gameStatus.settings.populate = false;
        if (gameStatus.settings.paused) {
            hudManager->setPrompt("Game is paused!");
        } else {
            hudManager->setPrompt("");
        }
    }

    float deltaT = handleMovement();

    inputManager->update(gameStatus, deltaT);

    std::ranges::for_each(sceneHandlers, [&](auto &handler) { handler->checkInteractionReach(gameStatus); });

    sceneLoader->updateUniformBuffer(currentImage, gameStatus, cameraController);
    hudManager->updateUniformBuffer(currentImage, deltaT);
}

void Game::localCleanup() {
    sceneLoader->cleanUp();
    hudManager->cleanUp();
    hudManagerNew->cleanUp();
}

void Game::pipelinesAndDescriptorSetsCleanup() {
    sceneLoader->pipelinesAndDescriptorSetsCleanup();
    hudManager->pipelinesAndDescriptorSetsCleanup();
    hudManagerNew->pipelinesAndDescriptorSetsCleanup();
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
    sceneLoader->populateCommandBuffer(commandBuffer, currentImage);
    if (gameStatus.settings.paused) {
        hudManagerNew->populateCommandBuffer(commandBuffer, currentImage);
    }
}

float Game::handleMovement() {
    float deltaT;
    auto m = glm::vec3(0.0f);
    auto r = glm::vec3(0.0f);
    bool fire = false;
    getSixAxis(deltaT, m, r, fire);

    if (!gameStatus.settings.paused) {
        glm::vec3 newPos = cameraController.computePos(gameStatus.player.pos, m, r, deltaT);
        gameStatus.player.pos = sceneLoader->getCollisionDetector()->resolveCollision(gameStatus.player, newPos);
        cameraController.updateViewPrjMat(gameStatus.player.pos);
        gameStatus.player.updateColliders();
    }

    return deltaT;
}
