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
    alignas(16) glm::vec3 lightParams;
    alignas(16) glm::vec3 playerPos;
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
