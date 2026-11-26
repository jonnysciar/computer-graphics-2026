#pragma once

struct Instance;
#include "GameStatus.hpp"

namespace HandlerHelper {
    void hideInstance(Instance *instance);
    void sendInstanceToCoordinates(Instance *instance, float x, float y, float z);
    bool isTorchPuzzleSatisfied(std::vector<Torch> torches);
} // namespace HandlerHelper
