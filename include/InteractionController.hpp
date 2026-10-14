#pragma once

#include "CollisionDetector.hpp"

#include <string>
#include <vector>

// An object the player can interact with (e.g. pick up)
struct Interactable {
    std::string id;     // instance id in scene.json
    Collider *collider; // not owned, the scene owns it
    bool taken = false;
};

// Decides which interactable object is in reach and when it gets picked up.
// It does not know the scene: the caller applies the effect of the interaction.
class InteractionController {
private:
    std::vector<Interactable> items;
    std::string focusedId; // empty if nothing is in reach

public:
    void add(const std::string &id, Collider *collider);

    // Returns true if an object has been picked up this frame (its id is stored in pickedId)
    bool update(CollisionDetector &detector, bool interactPressed, std::string &pickedId);

    // Id of the object currently in reach (empty if none)
    const std::string &focused() const { return focusedId; }
};
