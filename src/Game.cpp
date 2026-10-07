// This has been adapted from the Vulkan tutorial

#include "Game.hpp"
#include "CameraController.hpp"

// What to do when the window changes size
void Game::onWindowResize(int w, int h) {
    std::cout << "Window resized to: " << w << " x " << h << "\n";
    Ar = (float) w / (float) h;
    sceneManager->onResize(w, h);
    if (inputManager) {
        inputManager->updateWindowSize(w, h);
    }
}

// Here you load and setup all your Vulkan Models and Texutures.
// Here you also create your Descriptor set layouts and load the shaders for the pipelines
void Game::localInit() {
    inputManager = std::make_unique<InputManager>(window, windowWidth, windowHeight);
    inputManager->registerKey("change_texture", GLFW_KEY_3);
    inputManager->registerKey("quit", GLFW_KEY_ESCAPE);
    sceneManager = std::make_unique<SceneManager>(this, "", windowWidth, windowHeight);
}

// Here you create your pipelines and Descriptor Sets!
void Game::pipelinesAndDescriptorSetsInit() {
    sceneManager->pipelinesAndDescriptorSetsInit();
}

float Game::GameLogic(){
    const float FOVy = glm::radians(45.0f);
    const float nearPlane = 0.1f;
    const float farPlane = 100.f;
    const float MOVE_SPEED = 5.0f;
    const float ROT_SPEED = glm::radians(120.0f);

    float deltaT;
    glm::vec3 m = glm::vec3(0.0f);
    glm::vec3 r = glm::vec3(0.0f);
    bool fire = false;
    getSixAxis(deltaT, m, r, fire);

    glm::mat4* ViewPrj_ptr = &ViewPrj;
    glm::vec3* cameraPos_ptr = &cameraPos;
    float* mx_ptr = &m.x;
    float* my_ptr = &m.y;
    float* mz_ptr = &m.z;
    float* rx_ptr = &r.x;
    float* ry_ptr = &r.y;
    float* rz_ptr = &r.z;

    //*
    CameraController CC;
    deltaT = CC.CameraLogic(FOVy, nearPlane, farPlane, Ar, ROT_SPEED, MOVE_SPEED, deltaT, mx_ptr, my_ptr, mz_ptr, rx_ptr, ry_ptr, rz_ptr, cameraPos_ptr, ViewPrj_ptr);
    //*/

    /*
    static glm::vec3 camPos = glm::vec3(0.0f, 1.5f, 10.0f);
    static float Yaw   = glm::radians(0.0f);
    static float Pitch = 0.0f;
    static float Roll  = 0.0f;

    Yaw   += ROT_SPEED * deltaT * r.y;
    Pitch += ROT_SPEED * deltaT * r.x;
    Pitch  = glm::clamp(Pitch, glm::radians(-89.0f), glm::radians(89.0f));

    glm::vec3 forward = glm::normalize(glm::vec3(
        sin(Yaw) * cos(Pitch),
        -sin(Pitch),
        -cos(Yaw) * cos(Pitch)));
    glm::vec3 walkForward = glm::normalize(glm::vec3(sin(Yaw), 0.0f, -cos(Yaw)));
    glm::vec3 right = glm::normalize(glm::cross(walkForward, glm::vec3(0, 1, 0)));

    camPos += walkForward * MOVE_SPEED * deltaT * (-m.z);
    camPos += right       * MOVE_SPEED * deltaT *   m.x;

    cameraPos = camPos;

    glm::mat4 Prj = glm::perspective(FOVy, Ar, nearPlane, farPlane);
    Prj[1][1] *= -1;

    glm::mat4 View = glm::lookAt(camPos, camPos + forward, glm::vec3(0, 1, 0));
    ViewPrj = Prj * View;
    //*/

    return deltaT;

}

// Here is where you update the uniforms.
void Game::updateUniformBuffer(uint32_t currentImage) {
    // press 3 to change the texture
    if (inputManager->isKeyPressed("change_texture")) {
        debugView.z += 1.0f;
        if (debugView.z > 3.5f) {
            debugView.z = 0.0f;
        }
    }
    if (inputManager->isKeyDown("quit")) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    float deltaT = GameLogic();
    inputManager->update(deltaT);

    sceneManager->updateUniformBufferObjects(currentImage, debugView, ViewPrj, cameraPos, deltaT);
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

    // Initial aspect ratio
    Ar = 4.0f / 3.0f;
}
