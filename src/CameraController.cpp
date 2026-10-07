#include "CameraController.hpp"

float CameraController::CameraLogic (float FOVy, float nearPlane, float farPlane, float Ar, float ROT_SPEED, float MOVE_SPEED, float deltaT, float* mx, float* my, float* mz, float* rx, float* ry, float* rz, glm::vec3* cameraPos_ptr, glm::mat4* ViewPrj_ptr) {
    static glm::vec3 camPos = glm::vec3(0.0f, 1.5f, 10.0f);
    static float Yaw   = glm::radians(0.0f);
    static float Pitch = 0.0f;
    static float Roll  = 0.0f;

    Yaw   += ROT_SPEED * deltaT * *ry; //y
    Pitch += ROT_SPEED * deltaT * *rx; //x
    Pitch  = glm::clamp(Pitch, glm::radians(-89.0f), glm::radians(89.0f));

    glm::vec3 forward = glm::normalize(glm::vec3(
        sin(Yaw) * cos(Pitch),
        -sin(Pitch),
        -cos(Yaw) * cos(Pitch)));
    glm::vec3 walkForward = glm::normalize(glm::vec3(sin(Yaw), 0.0f, -cos(Yaw)));
    glm::vec3 right = glm::normalize(glm::cross(walkForward, glm::vec3(0, 1, 0)));

    camPos += walkForward * MOVE_SPEED * deltaT * -(*mz); //z
    camPos += right       * MOVE_SPEED * deltaT *   *mx;  //x

    *cameraPos_ptr = camPos;

    glm::mat4 Prj = glm::perspective(FOVy, Ar, nearPlane, farPlane);
    Prj[1][1] *= -1;

    glm::mat4 View = glm::lookAt(camPos, camPos + forward, glm::vec3(0, 1, 0));
    *ViewPrj_ptr = Prj * View;

    return deltaT;
}