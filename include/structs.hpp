#pragma once

#include <glm/glm.hpp>

// The uniform buffer object used in this example
struct UniformBufferObject {
    alignas(16) glm::mat4 mvpMat;
    alignas(16) glm::mat4 mMat;
    alignas(16) glm::mat4 nMat;
};

struct GlobalUniformBufferObject {
    alignas(16) glm::vec3 lightDir;
    alignas(16) glm::vec4 lightColor;
    alignas(16) glm::vec3 eyePos;
    alignas(16) glm::vec4 ambientUpper; // xyz = sky / upper  color  (lU)
    alignas(16) glm::vec4 ambientLower; // xyz = ground / lower color (lD)
    alignas(16) glm::vec4 ambientDir; // xyz = "up" direction for blending (d)
    alignas(16) glm::vec4 debugView;
};

struct Vertex {
    glm::vec3 pos;
    glm::vec3 norm;
    glm::vec2 UV;
    glm::vec4 tan;
};
