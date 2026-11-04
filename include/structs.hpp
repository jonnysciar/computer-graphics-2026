#pragma once

#include <Player.hpp>
#include <Door.hpp>

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
/*
struct  Door {
    const std::string id;
    bool isLocked;
};
//*/

struct GameStatus {
    Player player{};
    std::vector<bool> lightStates;

    Door door_1_2{"door_1_2", false, true};
    Door door_1_4{"door_1_4", false, true};
    Door door_1_5{"door_1_5", false, true};
    Door door_2_3{"door_2_3", false, true};
    Door door_5_6{"door_5_6", false, true};
    Door door_6_7{"door_6_7", false, true};
    Door door_7_o{"door_7_o", false, true};
};
