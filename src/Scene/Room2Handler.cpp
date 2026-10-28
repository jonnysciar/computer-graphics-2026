#include "Scene/Room2Handler.hpp"

Room2Handler::Room2Handler(SceneLoader *sceneLoader) : SceneHandler(sceneLoader) {
    interactionController = std::make_unique<InteractionController>(collisionDetector, std::vector<Interactable>{});
}

void Room2Handler::onInteractionReach(std::string_view id, Player &p) {}

void Room2Handler::offInteractionReach(std::string_view id, Player &p) {}

void Room2Handler::onEvent(Player &p, Key k) {
    switch (k.glfw_key) {
    default:
        break;
    }
}