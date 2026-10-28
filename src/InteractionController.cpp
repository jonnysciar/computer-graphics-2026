#include "InteractionController.hpp"

#include "Scene/SceneHandler.hpp"

#include <modules/Scene.hpp>

InteractionController::InteractionController(CollisionDetector *collisionDetector,
                                             std::vector<Interactable> interactables)
    : collisionDetector(collisionDetector), interactables(std::move(interactables)) {}

void InteractionController::checkInteractionReach(SceneHandler *sceneHandler, GameStatus &gs) {
    for (auto &interactable : interactables) {
        auto isCol = gs.player.reachCollider.collidesWith(*interactable.instance->C);
        if (isCol && !interactable.reachable) {
            sceneHandler->onInteractionReach(*interactable.instance->id, gs);
            interactable.reachable = true;
        } else if (!isCol && interactable.reachable) {
            sceneHandler->offInteractionReach(*interactable.instance->id, gs);
            interactable.reachable = false;
        }
    }
}

void InteractionController::interact(GameStatus &gs) {
    for (auto &in : interactables) {
        if (!in.reachable)
            continue;

        in.callback(in, gs);
    }
}
