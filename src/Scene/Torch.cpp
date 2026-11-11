#include "Scene/Torch.hpp"

bool Torch::isTorch(std::string_view id) { return id.starts_with("torch_"); }

std::vector<std::string_view> Torch::ofRoom(const SceneLoader *sceneLoader, int room) {
    std::string prefix = "torch_" + std::to_string(room) + "_";
    std::vector<std::string_view> ids;
    for (const auto &light : sceneLoader->getLights()) {
        if (light.id.starts_with(prefix))
            ids.emplace_back(light.id);
    }
    return ids;
}

bool Torch::isOn(const SceneLoader *sceneLoader, const GameStatus &gs, std::string_view id) {
    return gs.lightStates[sceneLoader->getLightIndex(id)];
}
