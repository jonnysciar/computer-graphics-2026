#include "CollisionDetector.hpp"
#include "Player.hpp"
#include "modules/Colliders.hpp"

#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>

namespace {
bool containsPoint(const AABBextents &e, const glm::vec3 &p) {
    return p.x >= e.xMin && p.x <= e.xMax && p.y >= e.yMin && p.y <= e.yMax && p.z >= e.zMin && p.z <= e.zMax;
}

// World-space segment a->b against an AABB (slab method)
bool segmentIntersectsAABB(const glm::vec3 &a, const glm::vec3 &b, const AABBextents &e) {
    const float mn[3] = {e.xMin, e.yMin, e.zMin};
    const float mx[3] = {e.xMax, e.yMax, e.zMax};
    const glm::vec3 d = b - a;
    float tMin = 0.0f, tMax = 1.0f;

    for (int i = 0; i < 3; ++i) {
        if (std::abs(d[i]) < 1e-8f) { // segment parallel to this slab
            if (a[i] < mn[i] || a[i] > mx[i])
                return false;
        } else {
            const float inv = 1.0f / d[i];
            float t1 = (mn[i] - a[i]) * inv;
            float t2 = (mx[i] - a[i]) * inv;
            if (t1 > t2)
                std::swap(t1, t2);
            tMin = std::max(tMin, t1);
            tMax = std::min(tMax, t2);
            if (tMin > tMax)
                return false;
        }
    }
    return true;
}
} // namespace

bool CollisionDetector::isOccluded(const glm::vec3 &playerPos, const glm::vec3 &lightPos) const {
    return std::ranges::any_of(colliders, [&](const auto &pair) {
        auto *collider = pair.first;
        if (!pair.second || !collider)
            return false; // only solid colliders can hide a light

        const AABBextents box = collider->getExtents();

        // A box that contains an endpoint cannot be "in between"
        if (containsPoint(box, lightPos) || containsPoint(box, playerPos))
            return false;
        return segmentIntersectsAABB(lightPos, playerPos, box);
    });
}

bool CollisionDetector::isLightVisible(const glm::vec3 &playerPos, const glm::vec3 &lightPos,
                                       const float radius) const {
    const glm::vec3 toCenter = lightPos - playerPos;
    const float dist = glm::length(toCenter);

    // Orthonormal basis around the view direction
    const glm::vec3 dir = toCenter / dist;
    const glm::vec3 helper = std::abs(dir.y) < 0.99f ? glm::vec3(0, 1, 0) : glm::vec3(1, 0, 0);
    const glm::vec3 right = glm::normalize(glm::cross(dir, helper));
    const glm::vec3 up = glm::cross(right, dir);

    const float r = radius;              // stay inside the silhouette
    const float diagR = r * 0.70710678f; // scaled for diagonals to maintain radius r

    // Expanded sample set (9 points: center, 4 cardinal, 4 diagonal)
    const glm::vec3 samples[9] = {lightPos,
                                  lightPos + right * r,
                                  lightPos - right * r,
                                  lightPos + up * r,
                                  lightPos - up * r,
                                  lightPos + (right + up) * diagR,
                                  lightPos + (right - up) * diagR,
                                  lightPos + (-right + up) * diagR,
                                  lightPos + (-right - up) * diagR};

    // Visible if at least one sample can be seen from the eye.
    return std::ranges::any_of(samples, [&](const glm::vec3 &p) { return !isOccluded(p, playerPos); });
}

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
