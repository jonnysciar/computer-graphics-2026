#include "Scene/Room1Handler.hpp"

#include "HUDManager.hpp"
#include "Scene/HandlerHelper.hpp"
#include "Scene/Torch.hpp"

#include <modules/Scene.hpp>

Room1Handler::Room1Handler(SceneLoader *sceneLoader, HUDManager *hudManager) : SceneHandler(sceneLoader, hudManager) {
    interMap = {{"key_1", Interactable{.instance = nullptr, .callback = [this](Interactable &i, GameStatus &gs) {
                                           HandlerHelper::hideInstance(i.instance);
                                           gs.player.pickUp(*i.instance->id);
                                           this->hudManager->setStatus("Keys: " +
                                                                       std::to_string(gs.player.countItems()));
                                       }}}};

    for (auto torchId : Torch::ofRoom(sceneLoader, 1)) {
        interMap.insert({torchId, Interactable{.instance = nullptr, .callback = [this](Interactable &i, GameStatus &gs) {
                                                   int idx = this->sceneLoader->getLightIndex(*i.instance->id);
                                                   gs.lightStates[idx] = !gs.lightStates[idx];
                                                   onInteractionReach(*i.instance->id, gs);
                                               }}});
    }
    setupInteractionController();
}

void Room1Handler::onInteractionReach(std::string_view id, GameStatus &gs) {
    if (!interMap.contains(id))
        return;

    if (Torch::isTorch(id))
        hudManager->setPrompt(Torch::isOn(sceneLoader, gs, id) ? "Press E to extinguish" : "Press E to light");
    else
        hudManager->setPrompt("Press E to pick up");
}

void Room1Handler::offInteractionReach(std::string_view id, GameStatus &gs) {
    if (!interMap.contains(id))
        return;

    hudManager->setPrompt("");
}

void Room1Handler::onEvent(GameStatus &gs, Key k) {
    switch (k.glfw_key) {
    case GLFW_KEY_E:
        interactionController->interact(gs);
        sceneLoader->refreshColliders();
        break;
    default:
        break;
    }
}