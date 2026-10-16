#include "CollisionDetector.hpp"

#include <glm/gtc/matrix_transform.hpp>

CollisionDetector::CollisionDetector() {
    // The spheres are defined around the local origin, the world matrix moves them on the camera
    playerBody.initSphere(0.0f, 0.0f, 0.0f, BODY_RADIUS);
    playerReach.initSphere(0.0f, 0.0f, 0.0f, REACH_RADIUS);
}

CollisionDetector::CollisionDetector(std::vector<Collider *> solids) : solids(std::move(solids)) {
    playerBody.initSphere(0.0f, 0.0f, 0.0f, BODY_RADIUS);
    playerReach.initSphere(0.0f, 0.0f, 0.0f, REACH_RADIUS);
}

void CollisionDetector::addSolid(Collider *collider) {
    if (collider != nullptr) {
        solids.push_back(collider);
    }
}

bool CollisionDetector::collidesWithSolids(const glm::vec3 &pos) {
    playerBody.setWorldMatrix(glm::translate(glm::mat4(1.0f), pos));
    for (Collider *solid : solids) {
        if (playerBody.collidesWith(*solid)) {
            return true;
        }
    }
    return false;
}

glm::vec3 CollisionDetector::resolveMovement(const glm::vec3 &from, const glm::vec3 &to) {
    glm::vec3 result = from;

    // One axis at a time: if an axis is blocked, the movement on the others is kept (sliding)
    for (int axis = 0; axis < 3; axis++) {
        glm::vec3 candidate = result;
        candidate[axis] = to[axis];
        if (!collidesWithSolids(candidate)) {
            result = candidate;
        }
    }

    return result;
}

void CollisionDetector::update(const glm::vec3 &cameraPos) {
    const glm::mat4 worldMatrix = glm::translate(glm::mat4(1.0f), cameraPos);
    playerBody.setWorldMatrix(worldMatrix);
    playerReach.setWorldMatrix(worldMatrix);
}

bool CollisionDetector::inReach(Collider &other) {
    return playerReach.collidesWith(other);
}
