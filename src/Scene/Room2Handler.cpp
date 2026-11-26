#include "Scene/Room2Handler.hpp"
#include "Hud/HudManager.hpp"

#include "Scene/HandlerHelper.hpp"

#include <modules/Scene.hpp>

Room2Handler::Room2Handler(SceneLoader *sceneLoader, HudManager *hudManager, GameStatus &gs)
    : SceneHandler(sceneLoader, hudManager) {
    for (auto &torch : gs.torches) {
        if (torch.room != 2 || !torch.interactable)
            continue;
        interMap.insert({torch.id, Interactable{.instance = nullptr, .callback = [&](Interactable &, GameStatus &gs) {
                                                    torch.on = !torch.on;
                                                    for (Instance *d14 : instances) {
                                                        if (*d14->id == "door_1_4") {
                                                            if (HandlerHelper::isTorchPuzzleSatisfied(gs.torches)) {
                                                                // std::cout << "[DEBUG] hideInstance next line\n" <<
                                                                // std::endl;
                                                                HandlerHelper::hideInstance(d14);
                                                                gs.door_1_4.setIsOpen(true);
                                                                gs.door_1_4.setIsLocked(false);
                                                            } else {
                                                                // std::cout << "[DEBUG] sendInstanceToCoordinates next
                                                                // line\n" << std::endl;
                                                                HandlerHelper::sendInstanceToCoordinates(d14, -16.1f,
                                                                                                         0.0f, 1.9f);
                                                                gs.door_1_4.setIsOpen(false);
                                                                gs.door_1_4.setIsLocked(true);
                                                            }
                                                        }
                                                    }
                                                }}});
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
