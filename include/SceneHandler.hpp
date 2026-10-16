#pragma once

#include "CollisionDetector.hpp"
#include "InteractionController.hpp"
#include "SceneLoader.hpp"
#include "Listener/Listener.hpp"
#include "InputManager.hpp"

#include <cstdint>
#include <vector>

struct Instance;

class SceneHandler : public Listener<Key>, Listener<MouseState> {
  protected:
    SceneLoader *sceneLoader;
    std::vector<Instance *> instances;
    std::unique_ptr<CollisionDetector> collisionDetector;
    std::unique_ptr<InteractionController> interactionController;

  public:
    explicit SceneHandler(SceneLoader *sceneLoader)
        : sceneLoader(sceneLoader), instances(this->sceneLoader->getInstances()) {
        auto colliders = instances | std::views::transform([](auto *inst) { return inst->C; });
        collisionDetector =
            std::make_unique<CollisionDetector>(std::vector<Collider *>{colliders.begin(), colliders.end()});
    }
    ~SceneHandler() override = default;

    virtual void updateUniformBuffers(uint32_t currentImage, glm::vec3 pos, glm::mat4 viewPrjMat, void *data) = 0;
    virtual void onInteractionReach(std::string_view id, Player &p) = 0;
    virtual void offInteractionReach(std::string_view id, Player &p) = 0;

    void checkInteractionReach(Player &p) {
        if (!interactionController) return;

        interactionController->checkInteractionReach(this, p);
    }

    glm::vec3 resolveCollision(Player &player, glm::vec3 newPos) const {
        return collisionDetector->resolveCollision(player, newPos);
    }

    void onEvent(Player &p, Key event) override {}
    void onEvent(Player &p, MouseState event) override {}
};
