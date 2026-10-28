#pragma once

#include <Player.hpp>

// The uniform buffer object used in this example
struct UniformBufferObject {
    alignas(16) glm::mat4 mvpMat;
    alignas(16) glm::mat4 mMat;
    alignas(16) glm::mat4 nMat;
};

struct GlobalUniformBufferObject {
    alignas(16) glm::vec4 lightColor;
    alignas(16) glm::vec3 pointParams;
    alignas(16) glm::vec3 eyePos;
    alignas(16) glm::vec4 ambientUpper; // xyz = sky / upper  color  (lU)
    alignas(16) glm::vec4 ambientLower; // xyz = ground / lower color (lD)
    alignas(16) glm::vec4 ambientDir;   // xyz = "up" direction for blending (d)
};

struct Vertex {
    glm::vec3 pos;
    glm::vec3 norm;
    glm::vec2 UV;
    glm::vec4 tan;
};

struct GameStatus {
    Player player{};
    std::vector<bool> lightStates;
};
