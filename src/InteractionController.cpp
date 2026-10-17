#include "InteractionController.hpp"

#include "Scene/SceneHandler.hpp"

#include <modules/Scene.hpp>

InteractionController::InteractionController(CollisionDetector *collisionDetector,
                                             std::vector<Interactable> interactables)
    : collisionDetector(collisionDetector), interactables(std::move(interactables)) {}

void InteractionController::checkInteractionReach(SceneHandler *sceneHandler, Player &player) {
    for (auto &interactable : interactables) {
        auto isCol = player.reachCollider.collidesWith(*interactable.instance->C);
        if (isCol && !interactable.reachable) {
            sceneHandler->onInteractionReach(*interactable.instance->id, player);
            interactable.reachable = true;
        } else if (!isCol && interactable.reachable) {
            sceneHandler->offInteractionReach(*interactable.instance->id, player);
            interactable.reachable = false;
        }
    }
}

void InteractionController::interact(Player &p) {
    for (auto &in : interactables) {
        if (!in.reachable)
            continue;

        in.callback(in, p);
    }
}
