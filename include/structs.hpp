#pragma once

#include <Player.hpp>

// The uniform buffer object used in this example
struct UniformBufferObject {
    alignas(16) glm::mat4 mvpMat;
    alignas(16) glm::mat4 mMat;
    alignas(16) glm::mat4 nMat;
    alignas(16) glm::vec4 emission;
};

// Must match MAX_LIGHTS in scene.frag
constexpr int MAX_LIGHTS = 64;

struct PointLight {
    alignas(16) glm::vec4 posRange; // xyz = position, w = range
    alignas(16) glm::vec4 colorInt; // rgb = color, a = intensity (0 = off)
};

struct GlobalUniformBufferObject {
    alignas(16) glm::vec3 playerPos;
    alignas(4) int lightCount;
    alignas(16) glm::vec4 decay; // x = g, y = beta
    PointLight lights[MAX_LIGHTS];
};

struct LightDef {
    std::string id;
    glm::vec3 pos;
    glm::vec3 color;
    float intensity;
    float range;
    bool on;
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
