#include "physics/CollisionSystem.hpp"

namespace Engine::Physics {

void CollisionSystem::addStaticBox(const BoundingBox& box) {
    m_staticBoxes.push_back(box);
}

void CollisionSystem::clear() {
    m_staticBoxes.clear();
}

bool CollisionSystem::checkCollision(const BoundingSphere& sphere, glm::vec3& outMTV) const {
    bool hasCollision = false;
    outMTV = glm::vec3(0.0f);

    for (const auto& box : m_staticBoxes) {
        glm::vec3 mtv(0.0f);
        if (sphere.getMTV(box, mtv)) {
            hasCollision = true;
            outMTV += mtv;
        }
    }

    return hasCollision;
}

bool CollisionSystem::checkCollision(const BoundingBox& box, glm::vec3& outMTV) const {
    bool hasCollision = false;
    outMTV = glm::vec3(0.0f);

    for (const auto& staticBox : m_staticBoxes) {
        glm::vec3 mtv(0.0f);
        if (box.getMTV(staticBox, mtv)) {
            hasCollision = true;
            outMTV += mtv;
        }
    }

    return hasCollision;
}

glm::vec3 CollisionSystem::resolveSpherePosition(const glm::vec3& currentPos, float radius, int maxIterations) const {
    glm::vec3 resolvedPos = currentPos;

    for (int iter = 0; iter < maxIterations; ++iter) {
        BoundingSphere testSphere(resolvedPos, radius);
        bool collisionFound = false;

        for (const auto& box : m_staticBoxes) {
            glm::vec3 mtv(0.0f);
            if (testSphere.getMTV(box, mtv)) {
                resolvedPos += mtv;
                testSphere.center = resolvedPos;
                collisionFound = true;
            }
        }

        if (!collisionFound) {
            break;
        }
    }

    return resolvedPos;
}

} // namespace Engine::Physics
