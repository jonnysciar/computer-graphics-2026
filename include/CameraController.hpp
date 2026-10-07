#pragma once

#include "modules/Starter.hpp"

class CameraController {
public:
    float CameraLogic (float FOVy, float nearPlane, float farPlane, float Ar, float ROT_SPEED, float MOVE_SPEED, float deltaT, float* mx, float* my, float* mz, float* rx, float* ry, float* rz, glm::vec3* cameraPos_ptr, glm::mat4* ViewPrj_ptr);
};