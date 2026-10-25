#pragma once

#include "physics/Collider.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <cmath>
#include <algorithm>

namespace Engine::Physics {

/**
 * @brief Controlador dinamico y cinematico del kart estilo arcade (Mario Kart).
 */
class KartPhysics {
public:
    KartPhysics();

    void update(float deltaTime, bool accelerate, bool brake, float steerInput, bool drift = false);

    void setPosition(const glm::vec3& pos) { m_position = pos; }
    [[nodiscard]] const glm::vec3& getPosition() const { return m_position; }

    void setYaw(float yawDeg) { m_yawDegrees = yawDeg; }
    [[nodiscard]] float getYaw() const { return m_yawDegrees; }

    [[nodiscard]] float getSpeed() const { return m_currentSpeed; }
    void resetSpeed() { m_currentSpeed = 0.0f; }
    [[nodiscard]] float getNormalizedSpeed() const { return std::abs(m_currentSpeed) / m_maxSpeed; }
    [[nodiscard]] bool isReversing() const { return m_currentSpeed < -0.1f; }
    [[nodiscard]] float getSteeringAngle() const { return m_steeringAngle; }
    [[nodiscard]] bool isDrifting() const { return m_isDrifting; }

    [[nodiscard]] glm::vec3 getForwardVector() const;

    [[nodiscard]] BoundingSphere getBoundingSphere() const {
        return BoundingSphere(m_position + glm::vec3(0.0f, 0.25f, 0.07f), 0.34f);
    }

    [[nodiscard]] BoundingBox getBoundingBox() const {
        return BoundingBox::fromCenterHalfExtents(m_position + glm::vec3(0.0f, 0.28f, 0.0f), glm::vec3(0.38f, 0.28f, 0.56f));
    }

    void applyBoost(float duration = 1.2f, float multiplier = 1.28f, float instantImpulse = 3.5f);
    [[nodiscard]] bool isBoosting() const { return m_boostTimer > 0.0f; }
    [[nodiscard]] float getBoostTimer() const { return m_boostTimer; }
    [[nodiscard]] float getBoostMultiplier() const { return m_boostMultiplier; }
    void setBoostMultiplier(float mult) { m_boostMultiplier = mult; }

    void setSurfaceFriction(float multiplier) { m_surfaceFriction = std::clamp(multiplier, 0.1f, 1.0f); }
    [[nodiscard]] float getSurfaceFriction() const { return m_surfaceFriction; }

    /**
     * @brief Resuelve impacto contra una barrera u obstaculo.
     * @param mtv Vector de separacion minimo obtenido del CollisionSystem.
     * @param reboundFactor Factor de rebote elastico (0.0 = deslizamiento seco, 0.3 = leve rebote).
     */
    void onCollision(const glm::vec3& mtv, float reboundFactor = 0.2f);

    // Parametros configurables
    float m_acceleration = 11.5f;
    float m_maxSpeed = 14.5f;
    float m_maxReverseSpeed = 4.5f;
    float m_drag = 5.0f;
    float m_brakingRate = 22.0f;
    float m_turnSpeed = 155.0f;
    float m_maxSteerAngle = 28.0f; // Grados visuales de giro de ruedas

private:
    glm::vec3 m_position{0.0f};
    float m_yawDegrees{0.0f};
    float m_currentSpeed{0.0f};
    float m_steeringAngle{0.0f};
    bool m_isDrifting{false};

    float m_surfaceFriction{1.0f};
    float m_boostTimer{0.0f};
    float m_boostMultiplier{1.28f};
};

} // namespace Engine::Physics
