#pragma once

#include "CameraController.hpp"
#include "HUDManager.hpp"
#include "InputManager.hpp"
#include "Player.hpp"
#include "Scene/SceneLoader.hpp"
#include "Scene/SceneHandler.hpp"

class Game : public BaseProject {
  protected:
    RenderPass renderPass;
    std::vector<std::unique_ptr<SceneLoader>> sceneLoaders;
    std::vector<std::unique_ptr<SceneHandler>> sceneHandlers;

    std::unique_ptr<HUDManager> hudManager;
    std::unique_ptr<InputManager> inputManager;

    CameraController cameraController;

    glm::mat4 viewPrjMat = glm::mat4(0.0f);

    Player player{};

    // What to do when the window changes size
    void onWindowResize(int w, int h) override;

    // Here you load and setup all your Vulkan Models and Texutures.
    // Here you also create your Descriptor set layouts and load the shaders for the pipelines
    void localInit() override;

    // Here you create your pipelines and Descriptor Sets!
    void pipelinesAndDescriptorSetsInit() override;

    void updateUniformBuffer(uint32_t currentImage) override;

    void localCleanup() override;

    // Here you destroy your pipelines and Descriptor Sets!
    void pipelinesAndDescriptorSetsCleanup() override;

  public:
    static void populateCommandBufferAccess(VkCommandBuffer commandBuffer, int currentImage, void *params);

    // Here you set the main application parameters
    void setWindowParameters() override;
    virtual ~Game() = default;

  private:
    void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage);
    float handleMovement();
};
