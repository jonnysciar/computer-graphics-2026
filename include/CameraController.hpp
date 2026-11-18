#pragma once

#include <glm/fwd.hpp>
#include <modules/Starter.hpp>

class CollisionDetector;

class CameraController {
  private:
    float Yaw = glm::radians(0.0f);
    float Pitch = 0.0f;
    float Roll = 0.0f;

    float ar = 4.0f / 3.0f;

    std::array<glm::vec4, 6> frustum;

    const float FOVy = glm::radians(45.0f);
    const float MOVE_SPEED = 10.0f;
    const float ROT_SPEED = glm::radians(120.0f);

  public:
    static const float nearPlane;
    static const float farPlane;

    glm::mat4 viewPrjMat = glm::mat4(1.0f);

    CameraController();

    // If collisionDetector is given, the movement is stopped by the solid colliders
    glm::vec3 computePos(glm::vec3 pos, glm::vec3 m, glm::vec3 r, float deltaT);
    void updateViewPrjMat(glm::vec3 pos);
    glm::mat4 computeAttachedModelMat(glm::vec3 pos, glm::vec3 localOffset) const;
    bool frustumIntersect(const glm::vec3 &c, float r) const;
    void onResize(int w, int h);

  private:
    glm::mat4 computeViewMat(glm::vec3 pos) const;
    void updateFrustum();
};
