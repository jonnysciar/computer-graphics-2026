#include "Scene/MainSceneHandler.hpp"

#include "HUDManager.hpp"

const std::map<std::string_view, Interactable> MainSceneHandler::interMap = {
    /*{"softball",
     Interactable{.instance = nullptr, .callback = [](Interactable &i, Player &p) { hideInstance(i.instance); }}},

    {"walls1", Interactable{.instance = nullptr, .callback = [](Interactable &i, Player &p) { p.pos.y += 1.0f; }}},*/
};

MainSceneHandler::MainSceneHandler(SceneLoader *sceneLoader, HUDManager *hudManager)
    : SceneHandler(sceneLoader), hudManager(hudManager) {

    std::vector<Interactable> interactables;
    interactables.reserve(interMap.size());
    for (const auto &instance : instances) {
        auto it = interMap.find(*instance->id);
        if (it == interMap.end())
            continue;

        auto inter = it->second;
        inter.instance = instance;
        interactables.push_back(std::move(inter));
    }

    interactionController = std::make_unique<InteractionController>(collisionDetector, std::move(interactables));
}

void MainSceneHandler::onInteractionReach(std::string_view id, Player &p) {
    if (!interMap.contains(id))
        return;

    hudManager->setPrompt("Press E to pick up");
}

void MainSceneHandler::offInteractionReach(std::string_view id, Player &p) {
    if (!interMap.contains(id))
        return;

    hudManager->setPrompt("");
}

void MainSceneHandler::onEvent(Player &p, Key k) {
    switch (k.glfw_key) {
    case GLFW_KEY_E:
        interactionController->interact(p);
        sceneLoader->refreshColliders();
        break;
    case GLFW_KEY_L:
        debugView.x = 0.5f;
        increase = true;
        break;
    case GLFW_KEY_M: {
        auto ins = instances | std::views::filter([](auto *inst) { return *inst->id == "softball"; });
        solid = !solid;
        std::ranges::for_each(ins, [this](auto &inst) { collisionDetector->addCollider(inst->C, solid); });
    } break;
    default:
        break;
    }
}