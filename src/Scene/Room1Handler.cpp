#include "Scene/Room1Handler.hpp"

Room1Handler::Room1Handler(SceneLoader *sceneLoader) : SceneHandler(sceneLoader) {
    interactionController = std::make_unique<InteractionController>(collisionDetector, std::vector<Interactable>{});
}

void Room1Handler::onInteractionReach(std::string_view id, Player &p) {}

void Room1Handler::offInteractionReach(std::string_view id, Player &p) {}

void Room1Handler::onEvent(Player &p, Key k) {
    switch (k.glfw_key) {
    default:
        break;
    }
}