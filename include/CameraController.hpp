#pragma once

#include "modules/Starter.hpp"

class CameraController {
private:
    float Yaw   = glm::radians(0.0f);
    float Pitch = 0.0f;
    float Roll  = 0.0f;

    float ar =  4.0f / 3.0f;

    const float FOVy = glm::radians(45.0f);
    const float nearPlane = 0.1f;
    const float farPlane = 100.f;
    const float MOVE_SPEED = 5.0f;
    const float ROT_SPEED = glm::radians(120.0f);
public:
    void update(glm::mat4 &viewPrjMatrix, glm::vec3 &cameraPos, glm::vec3 m, glm::vec3 r, float deltaT);
    void onResize(int w, int h);
};