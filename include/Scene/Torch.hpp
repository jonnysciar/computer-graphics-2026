#pragma once

#include "SceneLoader.hpp"

#include <string_view>
#include <vector>

// Read-only access to torch data; torch ids follow "torch_<room>_<n>"
class Torch {
  public:
    static bool isTorch(std::string_view id);

    static std::vector<std::string_view> ofRoom(const SceneLoader *sceneLoader, int room);

    static bool isOn(const SceneLoader *sceneLoader, const GameStatus &gs, std::string_view id);
};
