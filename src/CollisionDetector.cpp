#include "CollisionDetector.hpp"
#include "Player.hpp"

#include <glm/gtc/matrix_transform.hpp>

bool CollisionDetector::collidesWithSolids(Collider &body, const glm::vec3 &pos) {
    body.setWorldMatrix(glm::translate(glm::mat4(1.0f), pos));
    return std::ranges::any_of(colliders,
                               [&body](auto &pair) { return pair.second && body.collidesWith(*pair.first); });
}

CollisionDetector::CollisionDetector(std::map<Collider *, bool> colliders) : colliders(std::move(colliders)) {}

glm::vec3 CollisionDetector::resolveCollision(Player &player, const glm::vec3 &to) {
    glm::vec3 result = player.pos;

    // One axis at a time: if an axis is blocked, the movement on the others is kept (sliding)
    for (int axis = 0; axis < 3; axis++) {
        glm::vec3 candidate = result;
        candidate[axis] = to[axis];
        if (!collidesWithSolids(player.bodyCollider, candidate)) {
            result = candidate;
        }
    }

    return result;
}

void CollisionDetector::addCollider(Collider *c, bool isSolid) { colliders[c] = isSolid; }

void CollisionDetector::removeCollider(Collider *c) { colliders.erase(c); }
