#pragma once

struct Instance;
#include "GameStatus.hpp"

namespace HandlerHelper {
    void hideInstance(Instance *instance);
    bool isTorchPuzzleSatisfied(std::vector<Torch> torches);
} // namespace HandlerHelper
