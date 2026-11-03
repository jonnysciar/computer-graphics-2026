#include "Scene/Room4Handler.hpp"

#include "HUDManager.hpp"
#include "Scene/HandlerHelper.hpp"

#include <modules/Scene.hpp>

Room4Handler::Room4Handler(SceneLoader *sceneLoader, HUDManager *hudManager) : SceneHandler(sceneLoader, hudManager) {
    interMap = {{"key_3", Interactable{.instance = nullptr, .callback = [this](Interactable &i, GameStatus &gs) {
                                           HandlerHelper::hideInstance(i.instance);
                                           gs.player.pickUp(*i.instance->id);
                                           this->hudManager->setStatus("Keys: " +
                                                                       std::to_string(gs.player.countItems()));
                                       }}}};
    setupInteractionController();
}

void Room4Handler::onInteractionReach(std::string_view id, GameStatus &gs) {
    if (!interMap.contains(id))
        return;

    hudManager->setPrompt("Press E to pick up");
}

void Room4Handler::offInteractionReach(std::string_view id, GameStatus &gs) {
    if (!interMap.contains(id))
        return;

    hudManager->setPrompt("");
}

void Room4Handler::onEvent(GameStatus &gs, Key k) {
    switch (k.glfw_key) {
    case GLFW_KEY_E:
        interactionController->interact(gs);
        sceneLoader->refreshColliders();
        break;
    default:
        break;
    }
}
