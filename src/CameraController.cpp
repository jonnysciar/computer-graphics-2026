#include "CameraController.hpp"
#include "modules/Starter.hpp"
#include <glm/fwd.hpp>

const float CameraController::nearPlane = 0.1f;
const float CameraController::farPlane = 100.0f;

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

glm::mat4 CameraController::computeViewPrjMat(glm::vec3 pos, float nearPlane) const {
    glm::vec3 forward = glm::normalize(glm::vec3(sin(Yaw) * cos(Pitch), -sin(Pitch), -cos(Yaw) * cos(Pitch)));
    glm::mat4 Prj = glm::perspective(FOVy, ar, nearPlane, farPlane);
    Prj[1][1] *= -1;

    glm::mat4 View = glm::lookAt(pos, pos + forward, glm::vec3(0.0f, 1.0f, 0.0f));
    return Prj * View;
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

void CameraController::onResize(int w, int h) { ar = static_cast<float>(w) / static_cast<float>(h); }
