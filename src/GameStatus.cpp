#include "GameStatus.hpp"

// rooms reachable from room through at most depth doors (only the open ones if onlyOpen)
std::vector<int> GameStatus::roomsAround(int room, int depth, bool onlyOpen) {
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
std::vector<int> GameStatus::litRooms() { return roomsAround(playerRoom, LIGHT_DOOR_DEPTH, true); }

void GameStatus::loadLights(std::string_view jsonFile) {
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
                                .on = l.value("on", true),
                                .room = l.value("room", 0),
                                .interactable = l.value("interactable", true)});
    }
}

void GameStatus::onEvent(Key key) {
    switch (key.glfw_key) {
    case GLFW_KEY_ESCAPE:
        if (state == State::PAUSE) {
            state = State::GAME;
        } else if (state == State::GAME) {
            state = State::PAUSE;
        }
        break;
    case GLFW_KEY_1:
        settings.enableShadows = !settings.enableShadows;
        break;
    case GLFW_KEY_2:
        settings.shading = static_cast<Settings::Shading>((settings.shading + 1) % 3);
        std::cout << "Shading is " << settings.shading << std::endl;
        break;
    case GLFW_KEY_3:
        settings.dynamicLight = !settings.dynamicLight;
        std::cout << "Dynamic light is " << (settings.dynamicLight ? "enabled" : "disabled") << std::endl;
        break;
    case GLFW_KEY_4:
        settings.showFps = !settings.showFps;
        std::cout << "Show FPS is " << (settings.showFps ? "enabled" : "disabled") << std::endl;
        break;
    default:
        break;
    }
}
