#include "Scene/Room3Handler.hpp"
#include "Scene/HandlerHelper.hpp"
#include "Hud/HudManager.hpp"

#include <modules/Scene.hpp>

Room3Handler::Room3Handler(SceneLoader *sceneLoader, HudManager *hudManager) : SceneHandler(sceneLoader, hudManager) {
    interMap = {{"key_2", Interactable{.instance = nullptr,
                                       .callback =
                                           [](Interactable &i, GameStatus &gs) {
                                               HandlerHelper::hideInstance(i.instance);
                                               gs.player.pickUp(*i.instance->id);
                                           }}},
                {"door_2_3", Interactable{.instance = nullptr, .callback = [this, sceneLoader](Interactable &i, GameStatus &gs) {
                                              if (gs.door_2_3.getIsLocked() && !gs.player.hasItem("key_1")) {
                                                  this->hudManager->setPrompt("The door is locked");
                                              }
                                              if (!gs.door_2_3.getIsLocked()) {
                                                  if (gs.door_2_3.getIsOpen()) {
                                                      gs.door_2_3.rotateInstanceY(i.instance, -90.0f);
                                                      gs.door_2_3.setIsOpen(false);

                                                      glm::vec3 newPos = gs.player.pos + glm::vec3(2.0f, 0.0f, 0.0f);
                                                      gs.player.pos = sceneLoader->getCollisionDetector()->CollisionDetector::resolveCollision(gs.player, newPos);
                                                  } else {
                                                      gs.door_2_3.rotateInstanceY(i.instance, +90.0f);
                                                      gs.door_2_3.setIsOpen(true);

                                                      glm::vec3 newPos = gs.player.pos + glm::vec3(0.0f, 0.0f, -2.0f);
                                                      gs.player.pos = sceneLoader->getCollisionDetector()->CollisionDetector::resolveCollision(gs.player, newPos);
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
        if (gs.door_2_3.getIsLocked()) {
            if (gs.player.hasItem("key_1")) {
                gs.door_2_3.setIsLocked(false);
            }
        }
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
