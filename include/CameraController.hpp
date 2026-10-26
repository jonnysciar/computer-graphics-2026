#pragma once

#include <modules/Starter.hpp>

class CollisionDetector;

class CameraController {
  private:
    float Yaw = glm::radians(0.0f);
    float Pitch = 0.0f;
    float Roll = 0.0f;

    float ar = 4.0f / 3.0f;

    const float FOVy = glm::radians(45.0f);
    const float nearPlane = 0.1f;
    const float farPlane = 100.f;
    const float MOVE_SPEED = 10.0f;
    const float ROT_SPEED = glm::radians(120.0f);

  public:
    // If collisionDetector is given, the movement is stopped by the solid colliders
    glm::vec3 computePos(glm::vec3 pos, glm::vec3 m, glm::vec3 r, float deltaT);
    glm::mat4 computeViewPrjMat(glm::vec3 pos) const;
    void onResize(int w, int h);
};