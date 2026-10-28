#include "Scene/Room1Handler.hpp"

Room1Handler::Room1Handler(SceneLoader *sceneLoader) : SceneHandler(sceneLoader) {
    interactionController = std::make_unique<InteractionController>(collisionDetector, std::vector<Interactable>{});

    interMap = {{"Wall_1_N", Interactable{.instance = nullptr, .callback = [](Interactable &i, Player &p) {
                                              std::cout << "################ JS Wall 1 ################" << std::endl;
                                          }}}};
    setupInteractionController();
}

void Room1Handler::onInteractionReach(std::string_view id, Player &p) {}

void Room1Handler::offInteractionReach(std::string_view id, Player &p) {}

void Room1Handler::onEvent(Player &p, Key k) {
    switch (k.glfw_key) {
    case GLFW_KEY_E:
        interactionController->interact(p);
        break;
    default:
        break;
    }
}