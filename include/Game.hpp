#pragma once

#include "CameraController.hpp"
#include "SceneManager.hpp"
#include "InputManager.hpp"
#include "GameLogicManager.hpp"

#include "modules/Starter.hpp"
#include "modules/TextMaker.hpp"


class Game : public BaseProject {
protected:
    // Other application parameters
    std::unique_ptr<SceneManager> sceneManager;
    std::unique_ptr<InputManager> inputManager;
    std::unique_ptr<GameLogicManager> gameLogicManager;

    // What to do when the window changes size
    void onWindowResize(int w, int h) override;

    // Here you load and setup all your Vulkan Models and Texutures.
    // Here you also create your Descriptor set layouts and load the shaders for the pipelines
    void localInit() override;

    // Here you create your pipelines and Descriptor Sets!
    void pipelinesAndDescriptorSetsInit() override;

    void updateUniformBuffer(uint32_t currentImage) override;

    void localCleanup() override;

    float GameLogic();

    // Here you destroy your pipelines and Descriptor Sets!
    void pipelinesAndDescriptorSetsCleanup() override;

public:
    // Here you set the main application parameters
    void setWindowParameters() override;
};
