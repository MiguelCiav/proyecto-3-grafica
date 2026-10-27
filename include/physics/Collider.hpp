#pragma once

#include <glm/glm.hpp>
#include <algorithm>
#include <cmath>

namespace physics {

/**
 * @brief Caja alineada con los ejes (Axis-Aligned Bounding Box).
 */
struct BoundingBox {
    glm::vec3 min{0.0f};
    glm::vec3 max{0.0f};

    BoundingBox() = default;
    BoundingBox(const glm::vec3& minPoint, const glm::vec3& maxPoint)
        : min(minPoint), max(maxPoint) {}

    static BoundingBox fromCenterHalfExtents(const glm::vec3& center, const glm::vec3& halfExtents) {
        return BoundingBox(center - halfExtents, center + halfExtents);
    }

    [[nodiscard]] glm::vec3 getCenter() const {
        return (min + max) * 0.5f;
    }

    [[nodiscard]] glm::vec3 getSize() const {
        return max - min;
    }

    [[nodiscard]] bool contains(const glm::vec3& p) const {
        return (p.x >= min.x && p.x <= max.x) &&
               (p.y >= min.y && p.y <= max.y) &&
               (p.z >= min.z && p.z <= max.z);
    }

    [[nodiscard]] glm::vec3 closestPoint(const glm::vec3& p) const {
        return glm::vec3(
            std::clamp(p.x, min.x, max.x),
            std::clamp(p.y, min.y, max.y),
            std::clamp(p.z, min.z, max.z)
        );
    }

    [[nodiscard]] bool intersects(const BoundingBox& other) const {
        return (min.x <= other.max.x && max.x >= other.min.x) &&
               (min.y <= other.max.y && max.y >= other.min.y) &&
               (min.z <= other.max.z && max.z >= other.min.z);
    }

    /**
     * @brief Calcula el Minimum Translation Vector (MTV) para separar esta caja de `other`.
     * @return true si hay colision, outMTV apunta en la direccion para alejar `this` de `other`.
     */
    bool getMTV(const BoundingBox& other, glm::vec3& outMTV) const {
        if (!intersects(other)) {
            outMTV = glm::vec3(0.0f);
            return false;
        }

        float overlapX1 = max.x - other.min.x;
        float overlapX2 = other.max.x - min.x;
        float overlapX = (overlapX1 < overlapX2) ? -overlapX1 : overlapX2;

        float overlapY1 = max.y - other.min.y;
        float overlapY2 = other.max.y - min.y;
        float overlapY = (overlapY1 < overlapY2) ? -overlapY1 : overlapY2;

        float overlapZ1 = max.z - other.min.z;
        float overlapZ2 = other.max.z - min.z;
        float overlapZ = (overlapZ1 < overlapZ2) ? -overlapZ1 : overlapZ2;

        float absX = std::abs(overlapX);
        float absY = std::abs(overlapY);
        float absZ = std::abs(overlapZ);

        if (absX <= absY && absX <= absZ) {
            outMTV = glm::vec3(overlapX, 0.0f, 0.0f);
        } else if (absY <= absX && absY <= absZ) {
            outMTV = glm::vec3(0.0f, overlapY, 0.0f);
        } else {
            outMTV = glm::vec3(0.0f, 0.0f, overlapZ);
        }

        return true;
    }
};

/**
 * @brief Esfera envolvente para entidades y camara.
 */
struct BoundingSphere {
    glm::vec3 center{0.0f};
    float radius{0.5f};

    BoundingSphere() = default;
    BoundingSphere(const glm::vec3& c, float r) : center(c), radius(r) {}

    [[nodiscard]] bool intersects(const BoundingSphere& other) const {
        float distSq = glm::dot(center - other.center, center - other.center);
        float radSum = radius + other.radius;
        return distSq <= (radSum * radSum);
    }

    bool getMTV(const BoundingSphere& other, glm::vec3& outMTV) const {
        glm::vec3 diff = center - other.center;
        float dist = glm::length(diff);
        float minDist = radius + other.radius;

        if (dist >= minDist) {
            outMTV = glm::vec3(0.0f);
            return false;
        }

        if (dist > 0.0001f) {
            glm::vec3 normal = diff / dist;
            outMTV = normal * (minDist - dist);
        } else {
            outMTV = glm::vec3(0.0f, minDist, 0.0f);
        }
        return true;
    }

    [[nodiscard]] bool intersects(const BoundingBox& box) const {
        glm::vec3 closest = box.closestPoint(center);
        float distSq = glm::dot(center - closest, center - closest);
        return distSq <= (radius * radius);
    }

    /**
     * @brief Calcula el MTV para repeler la esfera fuera de la caja AABB.
     * @return true si colisionan, outMTV apunta en la direccion que saca a la esfera.
     */
    bool getMTV(const BoundingBox& box, glm::vec3& outMTV) const {
        glm::vec3 closest = box.closestPoint(center);
        glm::vec3 diff = center - closest;
        float distSq = glm::dot(diff, diff);

        if (distSq > (radius * radius)) {
            outMTV = glm::vec3(0.0f);
            return false;
        }

        float dist = std::sqrt(distSq);
        if (dist > 0.0001f) {
            glm::vec3 normal = diff / dist;
            float penetration = radius - dist;
            outMTV = normal * penetration;
            return true;
        }

        // Si el centro de la esfera esta adentro de la caja AABB:
        float dLeft = center.x - box.min.x;
        float dRight = box.max.x - center.x;
        float dBottom = center.y - box.min.y;
        float dTop = box.max.y - center.y;
        float dBack = center.z - box.min.z;
        float dFront = box.max.z - center.z;

        float minDist = dLeft;
        glm::vec3 normal(-1.0f, 0.0f, 0.0f);

        if (dRight < minDist) { minDist = dRight; normal = glm::vec3(1.0f, 0.0f, 0.0f); }
        if (dBottom < minDist) { minDist = dBottom; normal = glm::vec3(0.0f, -1.0f, 0.0f); }
        if (dTop < minDist) { minDist = dTop; normal = glm::vec3(0.0f, 1.0f, 0.0f); }
        if (dBack < minDist) { minDist = dBack; normal = glm::vec3(0.0f, 0.0f, -1.0f); }
        if (dFront < minDist) { minDist = dFront; normal = glm::vec3(0.0f, 0.0f, 1.0f); }

        outMTV = normal * (minDist + radius);
        return true;
    }
};

} // namespace physics

namespace Engine::Physics {
    using BoundingBox = ::physics::BoundingBox;
    using BoundingSphere = ::physics::BoundingSphere;
}
