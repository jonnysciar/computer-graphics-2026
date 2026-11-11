#pragma once

#include <glm/glm.hpp>

#define NUM_LIGHTS 1

// The uniform buffer object used in this example
struct UniformBufferObject {
    alignas(16) glm::mat4 mvpMat;
    alignas(16) glm::mat4 mMat;
    alignas(16) glm::mat4 nMat;
};

struct GlobalUniformBufferObject {
    alignas(16) glm::vec4 lightColor[NUM_LIGHTS];
    alignas(16) glm::vec4 lightPos[NUM_LIGHTS];
    alignas(16) glm::vec3 playerPos;
};

struct ShadowUBO {
    alignas(16) glm::mat4 model;                    // caster world matrix
    alignas(16) glm::vec4 params;                   // x = near, y = far, z = light count
    alignas(16) glm::vec4 lightPos[NUM_LIGHTS];
};

struct Vertex {
    glm::vec3 pos;
    glm::vec3 norm;
    glm::vec2 UV;
    glm::vec4 tan;
};
