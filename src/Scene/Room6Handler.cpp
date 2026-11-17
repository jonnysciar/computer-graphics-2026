#include "Scene/Room6Handler.hpp"

#include "HUDManager.hpp"
#include "Scene/HandlerHelper.hpp"

#include <modules/Scene.hpp>

Room6Handler::Room6Handler(SceneLoader *sceneLoader, HUDManager *hudManager, GameStatus &gs)
    : SceneHandler(sceneLoader, hudManager) {
    interMap = {{"door_5_6", Interactable{.instance = nullptr,
                                          .callback =
                                              [this](Interactable &i, GameStatus &gs) {
                                                  if (!gs.door_5_6.getIsLocked()) {
                                                      if (gs.door_5_6.getIsOpen()) {
                                                          gs.door_5_6.rotateInstanceY(i.instance, +90.0f);
                                                          gs.door_5_6.setIsOpen(false);
                                                      } else {
                                                          gs.door_5_6.rotateInstanceY(i.instance, -90.0f);
                                                          gs.door_5_6.setIsOpen(true);
                                                      }
                                                  }
                                              }}},
                {"door_6_7", Interactable{.instance = nullptr, .callback = [this](Interactable &i, GameStatus &gs) {
                                              if (!gs.door_6_7.getIsLocked()) {
                                                  if (gs.door_6_7.getIsOpen()) {
                                                      gs.door_6_7.rotateInstanceY(i.instance, +90.0f);
                                                      gs.door_6_7.setIsOpen(false);
                                                  } else {
                                                      gs.door_6_7.rotateInstanceY(i.instance, -90.0f);
                                                      gs.door_6_7.setIsOpen(true);
                                                  }
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

void Room6Handler::onInteractionReach(std::string_view id, GameStatus &gs) {
    if (!interMap.contains(id))
        return;

    auto torch = gs.torches | std::views::filter([id](auto &torch) { return torch.room == 6 && torch.id == id; });
    if (!torch.empty()) {
        hudManager->setPrompt(torch.front().on ? "Press E to extinguish" : "Press E to light");
        return;
    }

    if (id == "door_5_6" || id == "door_6_7") {
        hudManager->setPrompt("Press E to open/close");
    } else {
        hudManager->setPrompt("Press E to pick up");
    }
}

void Room6Handler::offInteractionReach(std::string_view id, GameStatus &gs) {
    if (!interMap.contains(id))
        return;

    hudManager->setPrompt("");
}

void Room6Handler::onEvent(GameStatus &gs, Key k) {
    switch (k.glfw_key) {
    case GLFW_KEY_E:
        interactionController->interact(gs);
        sceneLoader->refreshColliders();
        break;
    default:
        break;
    }
}