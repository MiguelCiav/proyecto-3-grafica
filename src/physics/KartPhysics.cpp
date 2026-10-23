#include "physics/KartPhysics.hpp"

namespace Engine::Physics {

KartPhysics::KartPhysics()
    : m_position(0.0f), m_yawDegrees(0.0f), m_currentSpeed(0.0f), m_steeringAngle(0.0f) {}

glm::vec3 KartPhysics::getForwardVector() const {
    float rad = glm::radians(m_yawDegrees);
    return glm::vec3(std::sin(rad), 0.0f, std::cos(rad));
}

void KartPhysics::applyBoost(float duration, float multiplier, float instantImpulse) {
    m_boostTimer = duration;
    m_boostMultiplier = multiplier;
    float boostedMax = m_maxSpeed * m_boostMultiplier;
    m_currentSpeed = std::min(m_currentSpeed + instantImpulse, boostedMax);
}

void KartPhysics::update(float deltaTime, bool accelerate, bool brake, float steerInput) {
    if (deltaTime <= 0.0f) return;

    // 1. Control del boost temporal
    if (m_boostTimer > 0.0f) {
        m_boostTimer -= deltaTime;
        if (m_boostTimer < 0.0f) m_boostTimer = 0.0f;
    }

    float currentMaxSpeed = m_maxSpeed;
    if (m_boostTimer > 0.0f) {
        currentMaxSpeed *= m_boostMultiplier;
    }
    // Modificador de superficie (cesped, asfalto)
    currentMaxSpeed *= m_surfaceFriction;

    // 2. Aceleracion y frenado longitudinal
    if (accelerate && !brake) {
        m_currentSpeed += m_acceleration * (m_boostTimer > 0.0f ? 1.5f : 1.0f) * deltaTime;
        if (m_currentSpeed > currentMaxSpeed) {
            m_currentSpeed = currentMaxSpeed;
        }
    } else if (brake && !accelerate) {
        if (m_currentSpeed > 0.5f) {
            // Frenado activo
            m_currentSpeed -= m_brakingRate * deltaTime;
            if (m_currentSpeed < 0.0f) m_currentSpeed = 0.0f;
        } else {
            // Reversa
            m_currentSpeed -= (m_acceleration * 0.6f) * deltaTime;
            if (m_currentSpeed < -m_maxReverseSpeed) {
                m_currentSpeed = -m_maxReverseSpeed;
            }
        }
    } else {
        // Desaceleracion pasiva por friccion con el suelo y resistencia del aire
        float activeDrag = m_drag / m_surfaceFriction; // Mas desaceleracion en cesped
        if (m_currentSpeed > 0.0f) {
            m_currentSpeed -= activeDrag * deltaTime;
            if (m_currentSpeed < 0.0f) m_currentSpeed = 0.0f;
        } else if (m_currentSpeed < 0.0f) {
            m_currentSpeed += activeDrag * deltaTime;
            if (m_currentSpeed > 0.0f) m_currentSpeed = 0.0f;
        }
    }

    // Si la velocidad supera la velocidad maxima permitida por la superficie actual (ej: entro al pasto rapido)
    if (m_currentSpeed > currentMaxSpeed && m_boostTimer <= 0.0f) {
        m_currentSpeed -= m_drag * 2.0f * deltaTime;
    }

    // 3. Direccion y viraje angular (yaw)
    // El viraje solo es efectivo si el vehiculo esta en movimiento
    float speedRatio = std::clamp(std::abs(m_currentSpeed) / (m_maxSpeed * 0.5f), 0.0f, 1.0f);
    float directionMultiplier = (m_currentSpeed >= 0.0f) ? 1.0f : -1.0f; // Invertir viraje en reversa

    // steerInput: -1.0 (Izquierda), +1.0 (Derecha)
    float clampedSteer = std::clamp(steerInput, -1.0f, 1.0f);
    m_steeringAngle = clampedSteer * m_maxSteerAngle;

    if (std::abs(clampedSteer) > 0.01f && speedRatio > 0.05f) {
        // En nuestro sistema de coordenadas:
        // yaw=0 -> (0, 0, 1). steerInput > 0 (derecha) rota en sentido horario (yaw negativo o positivo)
        // sin(yaw): yaw positivo rota hacia +X (derecha del eje +Z).
        m_yawDegrees -= clampedSteer * m_turnSpeed * speedRatio * directionMultiplier * deltaTime;

        // Normalizar yaw a [-180, 180]
        while (m_yawDegrees > 180.0f) m_yawDegrees -= 360.0f;
        while (m_yawDegrees < -180.0f) m_yawDegrees += 360.0f;
    }

    // 4. Desplazamiento lineal
    glm::vec3 forward = getForwardVector();
    m_position += forward * (m_currentSpeed * deltaTime);
}

void KartPhysics::onCollision(const glm::vec3& mtv, float reboundFactor) {
    // Corregir posicion expulsando el kart del colisionador
    m_position += mtv;

    // Disminuir o rebotar la velocidad si el impacto fue frontal
    glm::vec3 forward = getForwardVector();
    glm::vec3 mtvDir = glm::normalize(mtv);
    float dot = glm::dot(forward, mtvDir);

    if (dot < -0.38f) {
        // Impacto frontal directo contra el obstaculo
        m_currentSpeed = -m_currentSpeed * reboundFactor;
    } else {
        // Roce lateral o deslizamiento suave junto a conos y barreras
        m_currentSpeed *= 0.94f;
    }
}

} // namespace Engine::Physics
