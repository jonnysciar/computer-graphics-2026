#include "Scene/MainSceneHandler.hpp"

#include "HUDManager.hpp"

MainSceneHandler::MainSceneHandler(SceneLoader *sceneLoader, HUDManager *hudManager)
    : SceneHandler(sceneLoader), hudManager(hudManager) {

    interMap = {{"cube", Interactable{.instance = nullptr, .callback = [](Interactable &i, Player &p) {
                                          std::cout << "################ JS cube ################" << std::endl;
                                      }}}};

    setupInteractionController();
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