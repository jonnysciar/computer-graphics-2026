#include "InteractionController.hpp"

void InteractionController::add(const std::string &id, Collider *collider) {
    if (collider == nullptr) {
        return;
    }
    items.push_back({id, collider, false});
}

bool InteractionController::update(CollisionDetector &detector, bool interactPressed, std::string &pickedId) {
    focusedId.clear();

    // The first object in reach is the one the player is interacting with
    Interactable *focused = nullptr;
    for (auto &item : items) {
        if (!item.taken && detector.inReach(*item.collider)) {
            focused = &item;
            break;
        }
    }

    if (focused == nullptr) {
        return false;
    }

    if (interactPressed) {
        focused->taken = true;
        pickedId = focused->id;
        return true;
    }

    focusedId = focused->id;
    return false;
}
