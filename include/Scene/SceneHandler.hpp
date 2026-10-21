#pragma once

#include "CollisionDetector.hpp"
#include "InputManager.hpp"
#include "InteractionController.hpp"
#include "Listener/Listener.hpp"
#include "SceneLoader.hpp"

#include <cstdint>
#include <vector>

struct Instance;

class SceneHandler : public Listener<Player &, Key>, Listener<Player &, MouseState> {
  protected:
    SceneLoader *sceneLoader;
    std::vector<Instance *> instances;
    CollisionDetector *collisionDetector;
    std::unique_ptr<InteractionController> interactionController;

  public:
    explicit SceneHandler(SceneLoader *sceneLoader)
        : sceneLoader(sceneLoader), instances(this->sceneLoader->getInstances()),
          collisionDetector(this->sceneLoader->getCollisionDetector()) {}
    ~SceneHandler() override = default;

    virtual void updateUniformBuffers(uint32_t currentImage, Player &p, glm::mat4 viewPrjMat, float deltaT,
                                      void *data) = 0;
    virtual void onInteractionReach(std::string_view id, Player &p) = 0;
    virtual void offInteractionReach(std::string_view id, Player &p) = 0;

    void checkInteractionReach(Player &p) {
        if (!interactionController)
            return;

        interactionController->checkInteractionReach(this, p);
    }

    glm::vec3 resolveCollision(Player &player, glm::vec3 newPos) const {
        return collisionDetector->resolveCollision(player, newPos);
    }

    void onEvent(Player &p, Key event) override {}
    void onEvent(Player &p, MouseState event) override {}
};
