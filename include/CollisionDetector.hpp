#pragma once

#include "modules/Starter.hpp"
#include "modules/Colliders.hpp"

#include <vector>

// Keeps the player's colliders aligned with the camera and answers collision queries.
// It does not know anything about the scene: it only works with Collider objects.
class CollisionDetector {
private:
    static constexpr float BODY_RADIUS = 0.4f;
    static constexpr float REACH_RADIUS = 2.0f;

    Collider playerBody;  // physical volume of the player
    Collider playerReach; // area inside which the player can interact with objects

    std::vector<Collider *> solids; // colliders the player cannot walk through (not owned)

    // True if the player's body, placed at pos, touches any solid
    bool collidesWithSolids(const glm::vec3 &pos);

public:
    CollisionDetector();

    // Registers a collider the player cannot walk through
    void addSolid(Collider *collider);

    // Given the position before and after a movement, returns the position the player can actually reach.
    // Axes are tried one at a time, so the player slides along the surfaces instead of stopping.
    glm::vec3 resolveMovement(const glm::vec3 &from, const glm::vec3 &to);

    // To be called every frame with the new camera position
    void update(const glm::vec3 &cameraPos);

    // True if the given collider is inside the player's interaction area
    bool inReach(Collider &other);
};
