#include "Scene/Room3Handler.hpp"

#include "HUDManager.hpp"
#include "Scene/HandlerHelper.hpp"

#include <modules/Scene.hpp>

Room3Handler::Room3Handler(SceneLoader *sceneLoader, HUDManager *hudManager) : SceneHandler(sceneLoader, hudManager) {
    interMap = {{"key_2", Interactable{.instance = nullptr,
                                       .callback =
                                           [this](Interactable &i, GameStatus &gs) {
                                               HandlerHelper::hideInstance(i.instance);
                                               gs.player.pickUp(*i.instance->id);
                                               this->hudManager->setStatus("Keys: " +
                                                                           std::to_string(gs.player.countItems()));
                                           }}},
                {"door_2_3", Interactable{.instance = nullptr, .callback = [this](Interactable &i, GameStatus &gs) {
                                              if (!gs.door_2_3.getIsLocked()) {
                                                  if (gs.door_2_3.getIsOpen()) {
                                                      gs.door_2_3.rotateInstanceY(i.instance, -90.0f);
                                                      gs.door_2_3.setIsOpen(false);
                                                  } else {
                                                      gs.door_2_3.rotateInstanceY(i.instance, +90.0f);
                                                      gs.door_2_3.setIsOpen(true);
                                                  }
                                              }
                                          }}}};
    setupInteractionController();
}

void Room3Handler::onInteractionReach(std::string_view id, GameStatus &gs) {
    if (!interMap.contains(id))
        return;

    if (id == "door_2_3") {
        hudManager->setPrompt("Press E to open/close");
    } else {
        hudManager->setPrompt("Press E to pick up");
    }
}

void Room3Handler::offInteractionReach(std::string_view id, GameStatus &gs) {
    if (!interMap.contains(id))
        return;

    hudManager->setPrompt("");
}

void Room3Handler::onEvent(GameStatus &gs, Key k) {
    switch (k.glfw_key) {
    case GLFW_KEY_E:
        interactionController->interact(gs);
        sceneLoader->refreshColliders();
        break;
    default:
        break;
    }
}
