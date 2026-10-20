#pragma once

#include "CollisionDetector.hpp"

#include <vector>

struct Instance;
class SceneHandler;

// An object the player can interact with (e.g. pick up)
struct Interactable {
    Instance *instance;
    bool reachable{false};
    std::function<void(Interactable &, Player &)> callback;
};

// Decides which interactable object is in reach and when it gets picked up.
// It does not know the scene: the caller applies the effect of the interaction.
class InteractionController {
  private:
    CollisionDetector *collisionDetector;
    std::vector<Interactable> interactables;

  public:
    explicit InteractionController(CollisionDetector *collisionDetector, std::vector<Interactable> interactables);

    void checkInteractionReach(SceneHandler *sceneHandler, Player &player);
    void interact(Player &p);
};
