#include "physics/CollisionSolver.hpp"
#include "physics/CollisionSystem.hpp"
#include "physics/KartPhysics.hpp"
#include <algorithm>
#include <cmath>

namespace physics {

CollisionHit CollisionSolver::solveSphereAgainstSystem(const BoundingSphere& sphere, const CollisionSystem& system) {
    CollisionHit hit;
    glm::vec3 mtv(0.0f);

    if (system.checkCollision(sphere, mtv)) {
        hit.collided = true;
        hit.hasCollision = true;
        hit.mtv = mtv;
        float len = glm::length(mtv);
        if (len > 0.00001f) {
            hit.normal = mtv / len;
            hit.penetration = len;
        } else {
            hit.normal = glm::vec3(0.0f, 1.0f, 0.0f);
            hit.penetration = 0.0f;
        }
    } else {
        hit.collided = false;
        hit.hasCollision = false;
        hit.normal = glm::vec3(0.0f);
        hit.mtv = glm::vec3(0.0f);
        hit.penetration = 0.0f;
    }

    return hit;
}

glm::vec3 CollisionSolver::computeElasticResponse(const glm::vec3& currentVelocity, const glm::vec3& collisionNormal, float restitution) {
    float normalLenSq = glm::dot(collisionNormal, collisionNormal);
    if (normalLenSq < 1e-6f) {
        return currentVelocity;
    }

    glm::vec3 n = (std::abs(normalLenSq - 1.0f) > 1e-4f) ? glm::normalize(collisionNormal) : collisionNormal;
    float vDotN = glm::dot(currentVelocity, n);

    // Solo rebotar si la velocidad esta dirigida hacia la superficie (en sentido opuesto a la normal)
    if (vDotN < 0.0f) {
        return currentVelocity - (1.0f + restitution) * vDotN * n;
    }

    return currentVelocity;
}

glm::vec3 CollisionSolver::resolveCameraPosition(const glm::vec3& cameraPos,
                                                 float cameraRadius,
                                                 const CollisionSystem& system,
                                                 float minY,
                                                 float maxY) {
    // 1. Resolver iterativamente colisiones contra los obstaculos estaticos de la escena
    glm::vec3 resolvedPos = system.resolveSpherePosition(cameraPos, cameraRadius, 3);

    // 2. Clampar la coordenada Y dentro de [minY, maxY] para prevenir penetracion de suelo o desborde
    resolvedPos.y = std::clamp(resolvedPos.y, minY, maxY);

    return resolvedPos;
}

glm::vec3 CollisionSolver::resolveSpherePosition(const CollisionSystem& system,
                                                 const glm::vec3& position,
                                                 float radius,
                                                 int maxIterations) {
    return system.resolveSpherePosition(position, radius, maxIterations);
}

void CollisionSolver::resolveKartContact(KartPhysics& kart, const glm::vec3& mtv, float reboundFactor) {
    kart.onCollision(mtv, reboundFactor);
}

} // namespace physics
