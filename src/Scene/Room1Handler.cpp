#include "Scene/Room1Handler.hpp"
#include "Scene/HandlerHelper.hpp"
#include "Hud/HudManager.hpp"

Room1Handler::Room1Handler(SceneLoader *sceneLoader, HudManager *hudManager, GameStatus &gs)
    : SceneHandler(sceneLoader, hudManager) {
    interMap = {
        {"key_1", Interactable{.instance = nullptr,
                               .callback =
                                   [](Interactable &i, GameStatus &gs) {
                                       HandlerHelper::hideInstance(i.instance);
                                       gs.player.pickUp(*i.instance->id);
                                   }}},
        {"door_1_2",
         Interactable{.instance = nullptr,
                      .callback =
                          [sceneLoader](Interactable &i, GameStatus &gs) {
                              if (!gs.door_1_2.getIsLocked()) {
                                  if (gs.door_1_2.getIsOpen()) {
                                      gs.door_1_2.rotateInstanceY(i.instance, +90.0f);
                                      gs.door_1_2.setIsOpen(false);

                                      glm::vec3 newPos = gs.player.pos + glm::vec3(-2.0f, 0.0f, 0.0f);
                                      gs.player.pos =
                                          sceneLoader->getCollisionDetector()->CollisionDetector::resolveCollision(
                                              gs.player, newPos);
                                  } else {
                                      gs.door_1_2.rotateInstanceY(i.instance, -90.0f);
                                      gs.door_1_2.setIsOpen(true);

                                      glm::vec3 newPos = gs.player.pos + glm::vec3(0.0f, 0.0f, -2.0f);
                                      gs.player.pos =
                                          sceneLoader->getCollisionDetector()->CollisionDetector::resolveCollision(
                                              gs.player, newPos);
                                  }
                              }
                          }}},
        {"door_1_5",
         Interactable{.instance = nullptr, .callback = [sceneLoader](Interactable &i, GameStatus &gs) {
                          if (!gs.door_1_5.getIsLocked()) {
                              if (gs.door_1_5.getIsOpen()) {
                                  gs.door_1_5.rotateInstanceY(i.instance, -90.0f);
                                  gs.door_1_5.setIsOpen(false);

                                  glm::vec3 newPos = gs.player.pos + glm::vec3(0.0f, 0.0f, -2.0f);
                                  gs.player.pos =
                                      sceneLoader->getCollisionDetector()->CollisionDetector::resolveCollision(
                                          gs.player, newPos);
                              } else {
                                  gs.door_1_5.rotateInstanceY(i.instance, +90.0f);
                                  gs.door_1_5.setIsOpen(true);

                                  glm::vec3 newPos = gs.player.pos + glm::vec3(-2.0f, 0.0f, 0.0f);
                                  gs.player.pos =
                                      sceneLoader->getCollisionDetector()->CollisionDetector::resolveCollision(
                                          gs.player, newPos);
                              }
                          }
                      }}}};

    for (auto &torch : gs.torches) {
        if (torch.room != 1 || !torch.interactable)
            continue;
        interMap.insert({torch.id, Interactable{.instance = nullptr,
                                                .callback = [this, &torch](Interactable &inter, GameStatus &gs) {
                                                    torch.on = !torch.on;
                                                    onInteractionReach(*inter.instance->id, gs);
                                                }}});
    }
    setupInteractionController();
}

void Room1Handler::onInteractionReach(std::string_view id, GameStatus &gs) {
    if (!interMap.contains(id))
        return;

    auto torch = gs.torches | std::views::filter([id](auto &torch) { return torch.room == 1 && torch.id == id; });
    if (!torch.empty()) {
        hudManager->setPrompt(torch.front().on ? "Press E to extinguish" : "Press E to light");
        return;
    }
    if (id == "door_1_2" || id == "door_1_5") {
        hudManager->setPrompt("Press E to open/close");
        return;
    }
    if (id == "door_1_4") {
        if (HandlerHelper::isTorchPuzzleSatisfied(gs.torches)) {
            gs.door_1_4.setIsLocked(false);
            // hudManager->setPrompt("Press E to open/close");
        } else {
            gs.door_1_4.setIsLocked(true);
        }
        return;
    }
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
