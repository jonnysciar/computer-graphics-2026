// This has been adapted from the Vulkan tutorial

#include "Game.hpp"
#include "Scene/MainSceneHandler.hpp"
#include "Scene/Room1Handler.hpp"
#include "Scene/Room2Handler.hpp"
#include "Scene/Room3Handler.hpp"
#include "Scene/Room4Handler.hpp"
#include "Scene/Room6Handler.hpp"

// What to do when the window changes size
void Game::onWindowResize(int w, int h) {
    std::cout << "Window resized to: " << w << " x " << h << "\n";

    renderPass.width = w;
    renderPass.height = h;

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

    for (const auto &light : sceneLoader->getLights())
        gameStatus.lightStates.push_back(light.on);

    renderPass.init(this);
    submitCommandBuffer("main", 0, populateCommandBufferAccess, this);

    sceneHandlers.push_back(std::make_unique<MainSceneHandler>(sceneLoader.get(), hudManager.get()));
    sceneHandlers.push_back(std::make_unique<Room1Handler>(sceneLoader.get(), hudManager.get()));
    sceneHandlers.push_back(std::make_unique<Room3Handler>(sceneLoader.get(), hudManager.get()));
    sceneHandlers.push_back(std::make_unique<Room4Handler>(sceneLoader.get(), hudManager.get()));
    sceneHandlers.push_back(std::make_unique<Room6Handler>(sceneLoader.get(), hudManager.get()));

    inputManager = std::make_unique<InputManager>(window, windowWidth, windowHeight);
    inputManager->Listenable<Key>::addListener(gameStatus.player.getPtr());
    std::ranges::for_each(sceneHandlers, [&](auto &handler) {
        inputManager->Listenable<GameStatus &, Key>::addListener(
            handler->template Listener<GameStatus &, Key>::getPtr());
    });
}

// Here you create your pipelines and Descriptor Sets!
void Game::pipelinesAndDescriptorSetsInit() {
    renderPass.create();
    sceneLoader->pipelinesAndDescriptorSetsInit(renderPass);
    hudManager->pipelinesAndDescriptorSetsInit();
}

// Here is where you update the uniforms.
void Game::updateUniformBuffer(uint32_t currentImage) {
    float deltaT = handleMovement();

    inputManager->update(gameStatus, deltaT);

    std::ranges::for_each(sceneHandlers, [&](auto &handler) { handler->checkInteractionReach(gameStatus); });
    UniformBufferObject ubo{};
    GlobalUniformBufferObject gubo{};

    gubo.playerPos = gameStatus.player.pos;
    gubo.decay = glm::vec4(5.0f, 2.0f, 0.0f, 0.0f);

    const auto &lights = sceneLoader->getLights();
    gubo.lightCount = std::min(static_cast<int>(lights.size()), MAX_LIGHTS);
    for (int i = 0; i < gubo.lightCount; i++) {
        const auto &l = lights[i];
        gubo.lights[i].posRange = glm::vec4(l.pos, l.range);
        gubo.lights[i].colorInt = glm::vec4(l.color, gameStatus.lightStates[i] ? l.intensity : 0.0f);
    }

    const auto &instances = sceneLoader->getInstances();
    const auto &instanceLights = sceneLoader->getInstanceLights();

    // Each instance is drawn with its own world matrix (from scene.json), the same one used by its collider
    for (std::size_t k = 0; k < instances.size(); k++) {
        auto *instance = instances[k];
        ubo.mMat = instance->Wm;
        ubo.mvpMat = viewPrjMat * ubo.mMat;
        ubo.nMat = glm::inverse(glm::transpose(ubo.mMat));

        int li = instanceLights[k];
        if (li < 0)
            ubo.emission = glm::vec4(0.0f);
        else
            ubo.emission = gameStatus.lightStates[li] ? glm::vec4(lights[li].color, 1.0f) : glm::vec4(0.1f);

        instance->DS[0][0]->map(currentImage, &ubo, 0);
        instance->DS[0][0]->map(currentImage, &gubo, 1);
    }

    sceneLoader->updateUniformBuffer(currentImage, viewPrjMat);
    hudManager->updateUniformBuffer(currentImage, deltaT);
}

void Game::localCleanup() {
    sceneLoader->cleanUp();
    renderPass.destroy();
    hudManager->cleanUp();
}

void Game::pipelinesAndDescriptorSetsCleanup() {
    sceneLoader->pipelinesAndDescriptorSetsCleanup();
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
    sceneLoader->populateCommandBuffer(commandBuffer, currentImage);
    renderPass.end(commandBuffer);
}

float Game::handleMovement() {
    float deltaT;
    auto m = glm::vec3(0.0f);
    auto r = glm::vec3(0.0f);
    bool fire = false;
    getSixAxis(deltaT, m, r, fire);

    glm::vec3 newPos = cameraController.computePos(gameStatus.player.pos, m, r, deltaT);
    gameStatus.player.pos = sceneLoader->getCollisionDetector()->resolveCollision(gameStatus.player, newPos);
    viewPrjMat = cameraController.computeViewPrjMat(gameStatus.player.pos);
    gameStatus.player.updateColliders();

    return deltaT;
}
