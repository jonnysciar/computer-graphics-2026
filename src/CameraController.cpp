#include "CameraController.hpp"
#include "modules/Starter.hpp"
#include <glm/fwd.hpp>

const float CameraController::nearPlane = 0.1f;
const float CameraController::farPlane = 100.0f;

CameraController::CameraController() { updateFrustum(); }

glm::vec3 CameraController::computePos(glm::vec3 pos, glm::vec3 m, glm::vec3 r, float deltaT) {
    Yaw += ROT_SPEED * deltaT * r.y;   // y
    Pitch += ROT_SPEED * deltaT * r.x; // x
    Pitch = glm::clamp(Pitch, glm::radians(-89.0f), glm::radians(89.0f));

    glm::vec3 walkForward = glm::normalize(glm::vec3(sin(Yaw), 0.0f, -cos(Yaw)));
    glm::vec3 right = glm::normalize(glm::cross(walkForward, glm::vec3(0, 1, 0)));

    pos += walkForward * MOVE_SPEED * deltaT * -(m.z); // z
    pos += right * MOVE_SPEED * deltaT * m.x;          // x

    return pos;
}

void CameraController::updateViewPrjMat(glm::vec3 pos) {
    glm::mat4 Prj = glm::perspective(FOVy, ar, nearPlane, farPlane);
    Prj[1][1] *= -1;

    viewPrjMat = Prj * computeViewMat(pos);
    updateFrustum();
}

glm::mat4 CameraController::computeViewMat(glm::vec3 pos) const {
    glm::vec3 forward = glm::normalize(glm::vec3(sin(Yaw) * cos(Pitch), -sin(Pitch), -cos(Yaw) * cos(Pitch)));
    return glm::lookAt(pos, pos + forward, glm::vec3(0.0f, 1.0f, 0.0f));
}

glm::mat4 CameraController::computeAttachedModelMat(glm::vec3 pos, glm::vec3 offset) const {
    glm::mat4 localOffset = glm::translate(glm::mat4(1.0f), offset);
    localOffset = glm::rotate(localOffset, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 View = computeViewMat(pos);
    glm::mat4 cameraWorld = glm::inverse(View);
    return cameraWorld * localOffset;
}

void CameraController::updateFrustum() {
    auto row = [&](int r) { return glm::vec4(viewPrjMat[0][r], viewPrjMat[1][r], viewPrjMat[2][r], viewPrjMat[3][r]); };

    frustum[0] = row(3) + row(0); // left
    frustum[1] = row(3) - row(0); // right
    frustum[2] = row(3) + row(1); // bottom
    frustum[3] = row(3) - row(1); // top
    frustum[4] = row(2);          // near  (Vulkan: 0 <= z <= w)
    frustum[5] = row(3) - row(2); // far

    for (auto &p : frustum)
        p /= glm::length(glm::vec3(p)); // normalize so w is a real distance
}

bool CameraController::frustumIntersect(const glm::vec3 &c, float r) const {
    for (const auto &p : frustum)
        if (glm::dot(glm::vec3(p), c) + p.w < -r)
            return false; // fully outside this plane
    return true;
}

void CameraController::onResize(int w, int h) { ar = static_cast<float>(w) / static_cast<float>(h); }
