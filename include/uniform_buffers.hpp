#pragma once

#include <glm/glm.hpp>

// The uniform buffer object used in this example
struct UniformBufferObject {
    alignas(16) glm::mat4 mvpMat;
    alignas(16) glm::mat4 mMat;
    alignas(16) glm::mat4 nMat;
};

struct GlobalUniformBufferObject {
    alignas(16) glm::vec4 lightColor;
    alignas(16) glm::vec3 lightParams;
    alignas(16) glm::vec3 lightPos;
    alignas(16) glm::vec3 playerPos;
};

// std140-compatible: matches ShadowUBO in ShadowVert/ShadowFrag
struct ShadowUBO {
    alignas(16) glm::mat4 faceVP[6];
    alignas(16) glm::vec4 lightPosFar; // xyz = light position, w = far plane
    alignas(16) glm::mat4 model;
};

struct Vertex {
    glm::vec3 pos;
    glm::vec3 norm;
    glm::vec2 UV;
    glm::vec4 tan;
};
