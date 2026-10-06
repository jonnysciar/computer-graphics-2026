#include "CameraController.hpp"
#include "InputManager.hpp"

float CameraController::CameraLogic (float Ar, glm::mat4 ViewPrj, glm::vec3 cameraPos) {
    //*
    const float FOV = glm::radians(45.0f);
    const float nearPlane = 0.1f;
    const float farPlane = 100.f;
    const float moveSpeed = 5.0f;
    const float rotationSpeed = glm::radians(120.0f);
    InputManager IM;

    float deltaT;
    glm::vec3 m = glm::vec3(0.0f);
    glm::vec3 r = glm::vec3(0.0f);
    bool fire = false;
    IM.getSixAxis(deltaT, m, r, fire);

    float Yaw = glm::radians(0.0f); //angle that express the top-down movement of the camera
    float Pitch = 0.0f; //angle that express the left-right movement of the camera
    //static float Roll = 0.0f;

    static glm::vec3 camPos = glm::vec3(0.0f, 15.5f, 10.0f);

    Yaw   += rotationSpeed * deltaT * r.y;
    //modification of Yaw given time passed and input (r.y)
    Pitch += rotationSpeed * deltaT * r.x;
    //modification of Pitch given time passed and input (r.x)
    Pitch  = glm::clamp(Pitch, glm::radians(-89.0f), glm::radians(89.0f));
    //Yaw is not clamped because the character must be able to turn  up to 360 degrees

    //calculating the normal vector of the direction the character is currently looking to
    glm::vec3 forward = glm::normalize(glm::vec3(
            sin(Yaw) * cos(Pitch),
            -sin(Pitch),
            -cos(Yaw) * cos(Pitch)));
    glm::vec3 walkForward = glm::normalize(glm::vec3(sin(Yaw), 0.0f, -cos(Yaw)));
    glm::vec3 right = glm::normalize(glm::cross(walkForward, glm::vec3(0, 1, 0)));

    camPos += walkForward * moveSpeed * deltaT * (-m.z);
    camPos += right       * moveSpeed * deltaT *   m.x;

    cameraPos = camPos;

    glm::mat4 Prj = glm::perspective(FOV, Ar, nearPlane, farPlane);
    Prj[1][1] *= -1;

    glm::mat4 View = glm::lookAt(camPos, camPos + forward, glm::vec3(0, 1, 0));
    ViewPrj = Prj * View;

    return deltaT;
    //*/

    /*
    const float FOVy      = glm::radians(45.0f);
    const float nearPlane = 0.1f;
    const float farPlane  = 100.0f;
    const float ROT_SPEED  = glm::radians(120.0f);
    const float MOVE_SPEED = 5.0f;

    float deltaT;
    glm::vec3 m = glm::vec3(0.0f), r = glm::vec3(0.0f);
    bool fire = false;
    getSixAxis(deltaT, m, r, fire);

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

    return deltaT;
    //*/
}