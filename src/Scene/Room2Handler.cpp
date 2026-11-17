#include "Scene/Room2Handler.hpp"

#include "HUDManager.hpp"

#include <modules/Scene.hpp>

Room2Handler::Room2Handler(SceneLoader *sceneLoader, HUDManager *hudManager, GameStatus &gs)
    : SceneHandler(sceneLoader, hudManager) {
    for (auto &torch : gs.torches) {
        if (torch.room != 2 || !torch.interactable)
            continue;
        interMap.insert(
            {torch.id, Interactable{.instance = nullptr,
                                    .callback = [&torch](Interactable &, GameStatus &) { torch.on = !torch.on; }}});
    }
    setupInteractionController();
}

void Room2Handler::onInteractionReach(std::string_view id, GameStatus &gs) {
    if (!interMap.contains(id))
        return;

    auto torch = gs.torches | std::views::filter([id](auto &torch) { return torch.room == 2 && torch.id == id; });
    if (!torch.empty())
        hudManager->setPrompt(torch.front().on ? "Press E to extinguish" : "Press E to light");
}

void Room2Handler::offInteractionReach(std::string_view id, GameStatus &gs) {
    if (!interMap.contains(id))
        return;

    hudManager->setPrompt("");
}

void Room2Handler::onEvent(GameStatus &gs, Key k) {
    switch (k.glfw_key) {
    case GLFW_KEY_E:
        interactionController->interact(gs);
        break;
    default:
        break;
    }
}
