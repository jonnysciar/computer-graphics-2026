#pragma once

#include <Player.hpp>

#include <vector>

// Keeps the player's colliders aligned with the camera and answers collision queries.
// It does not know anything about the scene: it only works with Collider objects.
class CollisionDetector {
  private:
    std::vector<Collider *> colliders; // colliders the player cannot walk through (not owned)

    // True if the player's body, placed at pos, touches any solid
    bool collidesWithSolids(Collider &body, const glm::vec3 &pos);

  public:
    explicit CollisionDetector(std::vector<Collider *> solids);

    // Given the position before and after a movement, returns the position the player can actually reach.
    // Axes are tried one at a time, so the player slides along the surfaces instead of stopping.
    glm::vec3 resolveCollision(Player &player, const glm::vec3 &to);

    void addCollider(Collider *c);
    void removeCollider(Collider *c);
};
