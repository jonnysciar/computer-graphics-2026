#pragma once

#include "Player.hpp"
#include "Settings.hpp"
#include "Door.hpp"

struct Torch {
    static constexpr float CUTOFF = 1e-4f;
    std::string id;
    glm::vec3 pos;
    glm::vec3 color;
    float g;
    float beta;
    bool on;
    int room;
    bool interactable;

    float radius() {
        float maxC = glm::max(color.r, glm::max(color.g, color.b));
        return g * std::pow(maxC / CUTOFF, 1.0f / beta);
    }
};

class GameStatus {
  public:
    Player player{};
    Settings settings{};
    std::vector<Torch> torches;

    // the shadow maps must be rendered again (start, window resize, an object moved)
    bool shadowsDirty = true;

    // room the player is in (0 = not known yet), updated by SceneLoader
    int playerRoom = 0;

    Door door_1_2{"door_1_2", false, false};
    Door door_1_4{"door_1_4", true, false}; // secret mechanism
    Door door_1_5{"door_1_5", false, false};
    Door door_2_3{"door_2_3", true, false}; // key_1
    Door door_5_6{"door_5_6", true, false}; // key_2
    Door door_6_7{"door_6_7", false, false};
    Door door_7_o{"door_7_o", true, false}; // key_3

    // how many open doors the light of a torch can go through and still be seen by the player
    static constexpr int LIGHT_DOOR_DEPTH = 2;

    // rooms reachable from room through at most depth doors (only the open ones if onlyOpen)
    std::vector<int> roomsAround(int room, int depth, bool onlyOpen) {
        std::vector<int> rooms{room};
        std::size_t next = 0;
        for (int d = 0; d < depth; d++) {
            for (const std::size_t end = rooms.size(); next < end; next++) {
                for (Door *door : {&door_1_2, &door_1_4, &door_1_5, &door_2_3, &door_5_6, &door_6_7, &door_7_o}) {
                    const int other = door->otherRoom(rooms[next]);
                    const bool passable = door->getIsOpen() || !onlyOpen;
                    if (passable && other != -1 && std::ranges::find(rooms, other) == rooms.end())
                        rooms.push_back(other);
                }
            }
        }
        return rooms;
    }

    // rooms whose torches can light what the player sees: reachable through at most LIGHT_DOOR_DEPTH open doors
    std::vector<int> litRooms() { return roomsAround(playerRoom, LIGHT_DOOR_DEPTH, true); }

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
                                    .room = l.value("room", 0),
                                    .interactable = l.value("interactable", true)});
        }
    }
};
