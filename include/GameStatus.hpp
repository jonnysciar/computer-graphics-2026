#pragma once

#include "Player.hpp"
#include "Settings.hpp"
#include "Door.hpp"

struct Torch {
    std::string id;
    glm::vec3 pos;
    glm::vec3 color;
    float g;
    float beta;
    bool on;
    int room;
};

class GameStatus {
  public:
    Player player{};
    Settings settings{};
    std::vector<Torch> torches;

    Door door_1_2{"door_1_2", false, false};
    Door door_1_4{"door_1_4", false, false};
    Door door_1_5{"door_1_5", false, false};
    Door door_2_3{"door_2_3", false, false};
    Door door_5_6{"door_5_6", false, false};
    Door door_6_7{"door_6_7", false, false};
    Door door_7_o{"door_7_o", false, false};

    void loadLights(std::string_view jsonFile) {
        std::ifstream ifs(jsonFile.data());
        nlohmann::json json = nlohmann::json::parse(ifs);

        if (!json.contains("lights"))
            return;

        for (auto &l : json.at("lights")) {
            auto p = l.at("position").get<std::array<float, 3>>();
            auto c = l.at("color").get<std::array<float, 3>>();
            torches.push_back(Torch{.id = l.at("id").get<std::string>(),
                                    .pos = glm::vec3(p[0], p[1], p[2]),
                                    .color = glm::vec3(c[0], c[1], c[2]),
                                    .g = l.value("g", 10.0f),
                                    .beta = l.value("beta", 2.0f),
                                    .on = l.value("on", true),
                                    .room = l.value("room", 0)});
        }
    }
};
