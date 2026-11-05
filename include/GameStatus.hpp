#pragma once

#include <Player.hpp>
#include <Door.hpp>

struct GameStatus {
    Player player{};
    std::vector<bool> lightStates;

    Door door_1_2{"door_1_2", false, false};
    Door door_1_4{"door_1_4", false, false};
    Door door_1_5{"door_1_5", false, false};
    Door door_2_3{"door_2_3", false, false};
    Door door_5_6{"door_5_6", false, true};
    Door door_6_7{"door_6_7", false, true};
    Door door_7_o{"door_7_o", false, false};
};
