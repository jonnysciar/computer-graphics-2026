#include "Scene/Room7Handler.hpp"

#include "HUDManager.hpp"
#include "Scene/HandlerHelper.hpp"

#include <modules/Scene.hpp>

Room7Handler::Room7Handler(SceneLoader *sceneLoader, HUDManager *hudManager, GameStatus &gs) : SceneHandler(sceneLoader, hudManager) {
    interMap = {{"door_7_o", Interactable{.instance = nullptr, .callback = [this](Interactable &i, GameStatus &gs) {
        if ( gs.door_7_o.getIsLocked() && !gs.player.hasItem("key_3")) {
            this->hudManager->setPrompt("The door is locked");
        }
        if (!gs.door_7_o.getIsLocked()) {
            this->hudManager->setPrompt("CONGRATULATIONS!\nYou completed the Hunted Castle Game");
        }
    }}}};

    for (auto &torch : gs.torches) {
        if (torch.room != 6 || !torch.interactable)
            continue;
        interMap.insert(
            {torch.id, Interactable{.instance = nullptr,
                                    .callback = [&torch](Interactable &, GameStatus &) { torch.on = !torch.on; }}});
    }
    setupInteractionController();

}

void Room7Handler::onInteractionReach(std::string_view id, GameStatus &gs) {
    if (!interMap.contains(id))
        return;

    auto torch = gs.torches | std::views::filter([id](auto &torch) { return torch.room == 6 && torch.id == id; });
    if (!torch.empty()) {
        hudManager->setPrompt(torch.front().on ? "Press E to extinguish" : "Press E to light");
        return;
    }

    if (id == "door_7_o") {
        hudManager->setPrompt("Press E to open/close");
        if (gs.door_7_o.getIsLocked()) {
            if (gs.player.hasItem("key_3")) {
                gs.door_7_o.setIsLocked(false);
            }
        }
    } else {
        hudManager->setPrompt("Press E to pick up");
    }
}

void Room7Handler::offInteractionReach(std::string_view id, GameStatus &gs) {
    if (!interMap.contains(id))
        return;

    hudManager->setPrompt("");
}

void Room7Handler::onEvent(GameStatus &gs, Key k) {
    switch (k.glfw_key) {
    case GLFW_KEY_E:
        interactionController->interact(gs);
        sceneLoader->refreshColliders();
        break;
    default:
        break;
    }
}