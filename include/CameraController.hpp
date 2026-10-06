#pragma once

#include "modules/Starter.hpp"

class CameraController {
public:
    float CameraLogic (float Ar, glm::mat4 ViewPrj, glm::vec3 cameraPos);
};