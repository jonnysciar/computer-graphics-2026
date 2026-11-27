#pragma once

#include "Listener/Listener.hpp"
#include "Player.hpp"
#include "Door.hpp"

struct Settings {
    enum Shading {
        BASIC = 0,
        BLINN_PHONG = 1,
        COOK_TORRANCE = 2,
    };
    bool shadows = false;
    Shading shading = BASIC;
    int dynamicLight = false;
    bool showFps = false;

    std::string_view shadingString();
};

struct Torch {
    static constexpr float CUTOFF = 1e-4f;
    static constexpr float BETA = 2.0f;
    std::string id;
    glm::vec3 pos;
    glm::vec3 color;
    float g;
    bool on;
    int room;
    bool interactable;

    float radius() {
        float maxC = glm::max(color.r, glm::max(color.g, color.b));
        return g * std::pow(maxC / CUTOFF, 1.0f / BETA);
    }
};

class GameStatus : public Listener<Key> {
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

    enum class State { NONE, START, GAME, PAUSE, SETTINGS, END };
    State state{State::START};

    std::vector<int> roomsAround(int room, int depth, bool onlyOpen);
    std::vector<int> litRooms();
    void loadLights(std::string_view jsonFile);
    void onEvent(Key key) override;
};
