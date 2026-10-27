#include "physics/KartPhysics.hpp"

namespace physics {

KartPhysics::KartPhysics()
    : m_position(0.0f), m_yawDegrees(0.0f), m_currentSpeed(0.0f), m_steeringAngle(0.0f) {
    m_config.maxSpeed = m_maxSpeed;
    m_config.acceleration = m_acceleration;
    m_config.turnSpeed = m_turnSpeed;
    m_config.drag = m_drag;
    m_config.brakingRate = m_brakingRate;
    m_config.maxReverseSpeed = m_maxReverseSpeed;
    m_config.surfaceFriction = m_surfaceFriction;
    m_config.maxSteerAngle = m_maxSteerAngle;
}

void KartPhysics::setDynamicsConfig(const VehicleDynamicsConfig& config) {
    m_config = config;
    m_maxSpeed = config.maxSpeed;
    m_acceleration = config.acceleration;
    m_turnSpeed = config.turnSpeed;
    m_drag = config.drag;
    m_brakingRate = config.brakingRate;
    m_maxReverseSpeed = config.maxReverseSpeed;
    m_surfaceFriction = config.surfaceFriction;
    m_maxSteerAngle = config.maxSteerAngle;
}

const VehicleDynamicsConfig& KartPhysics::getDynamicsConfig() const {
    m_config.maxSpeed = m_maxSpeed;
    m_config.acceleration = m_acceleration;
    m_config.turnSpeed = m_turnSpeed;
    m_config.drag = m_drag;
    m_config.brakingRate = m_brakingRate;
    m_config.maxReverseSpeed = m_maxReverseSpeed;
    m_config.surfaceFriction = m_surfaceFriction;
    m_config.maxSteerAngle = m_maxSteerAngle;
    return m_config;
}

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

void KartPhysics::update(float deltaTime, bool accelerate, bool brake, float steerInput, bool drift) {
    if (deltaTime <= 0.0f) return;

    // 1. Control del boost temporal
    if (m_boostTimer > 0.0f) {
        m_boostTimer -= deltaTime;
        if (m_boostTimer < 0.0f) m_boostTimer = 0.0f;
    }

    bool hasBoost = (m_boostTimer > 0.0f);
    bool isOffRoad = (m_surfaceFriction < 0.95f) && !hasBoost;

    // Velocidad maxima efectiva (inmunidad al cesped durante turbo)
    float currentMaxSpeed = m_maxSpeed;
    if (hasBoost) {
        currentMaxSpeed *= m_boostMultiplier;
    } else {
        currentMaxSpeed *= m_surfaceFriction;
    }

    // Penalizacion firme si se excede la velocidad maxima de la superficie actual (ej: entrar al cesped a 50 km/h)
    if (isOffRoad && m_currentSpeed > currentMaxSpeed) {
        m_currentSpeed -= 28.0f * deltaTime;
        if (m_currentSpeed < currentMaxSpeed) {
            m_currentSpeed = currentMaxSpeed;
        }
    } else if (!hasBoost && m_currentSpeed > currentMaxSpeed) {
        m_currentSpeed -= m_drag * 2.0f * deltaTime;
        if (m_currentSpeed < currentMaxSpeed) {
            m_currentSpeed = currentMaxSpeed;
        }
    }

    // Aceleracion efectiva segun superficie
    float currentAccel = m_acceleration;
    if (hasBoost) {
        currentAccel *= 1.5f;
    } else if (isOffRoad) {
        currentAccel *= m_surfaceFriction;
    }

    // 2. Aceleracion y frenado longitudinal
    if (accelerate && !brake) {
        m_currentSpeed += currentAccel * deltaTime;
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
            float revAccel = currentAccel * 0.6f;
            m_currentSpeed -= revAccel * deltaTime;
            float maxRev = isOffRoad ? (m_maxReverseSpeed * m_surfaceFriction) : m_maxReverseSpeed;
            if (m_currentSpeed < -maxRev) {
                m_currentSpeed = -maxRev;
            }
        }
    } else {
        // Desaceleracion pasiva por friccion con el suelo y resistencia del aire
        float activeDrag = m_drag;
        if (isOffRoad) {
            activeDrag = m_drag / std::max(m_surfaceFriction, 0.1f);
        }
        if (m_currentSpeed > 0.0f) {
            m_currentSpeed -= activeDrag * deltaTime;
            if (m_currentSpeed < 0.0f) m_currentSpeed = 0.0f;
        } else if (m_currentSpeed < 0.0f) {
            m_currentSpeed += activeDrag * deltaTime;
            if (m_currentSpeed > 0.0f) m_currentSpeed = 0.0f;
        }
    }

    // 3. Direccion, viraje angular (yaw) y mecanica de Derrape (Power Slide / Handbrake)
    float clampedSteer = std::clamp(steerInput, -1.0f, 1.0f);
    m_steeringAngle = clampedSteer * m_maxSteerAngle;

    // Derrape activo si el jugador presiona ESPACIO/SHIFT o realiza viraje con freno activo (Handbrake Slide)
    bool handbrakeSlide = brake && (std::abs(clampedSteer) > 0.15f) && (m_currentSpeed > 2.0f);
    m_isDrifting = (drift || handbrakeSlide) && (std::abs(clampedSteer) > 0.1f);

    // "Scrubbing" dinamico de velocidad en curvas cerradas si no esta derrapando (ayuda a cerrar la trazada)
    if (std::abs(clampedSteer) > 0.6f && !m_isDrifting) {
        if (m_currentSpeed > 5.0f) {
            m_currentSpeed -= 6.5f * std::abs(clampedSteer) * deltaTime;
            if (m_currentSpeed < 5.0f) m_currentSpeed = 5.0f;
        }
    }

    // Viraje progresivo segun velocidad (con respuesta minima de 0.35 para no quedar atascado a baja velocidad)
    float speedRatio = std::clamp(std::abs(m_currentSpeed) / (m_maxSpeed * 0.35f), 0.35f, 1.0f);
    float directionMultiplier = (m_currentSpeed >= 0.0f) ? 1.0f : -1.0f; // Invertir viraje en reversa

    // Tasa de viraje: 220 deg/s base, incrementada a 1.40x (~308 deg/s) en modo derrape/handbrake
    float effectiveTurnSpeed = m_turnSpeed * (m_isDrifting ? 1.40f : 1.0f);

    if (std::abs(clampedSteer) > 0.01f) {
        // En nuestro sistema de coordenadas:
        // yaw=0 -> (0, 0, 1). steerInput > 0 (derecha) rota en sentido horario (yaw negativo)
        // sin(yaw): yaw positivo rota hacia +X (derecha del eje +Z).
        m_yawDegrees -= clampedSteer * effectiveTurnSpeed * speedRatio * directionMultiplier * deltaTime;

        // Normalizar yaw a [-180, 180]
        while (m_yawDegrees > 180.0f) m_yawDegrees -= 360.0f;
        while (m_yawDegrees < -180.0f) m_yawDegrees += 360.0f;
    }

    // 4. Desplazamiento lineal
    glm::vec3 forward = getForwardVector();
    m_position += forward * (m_currentSpeed * deltaTime);
}

void KartPhysics::onCollision(const glm::vec3& mtv, float reboundFactor) {
    // Corregir posicion expulsando el kart del colisionador (restringido al plano horizontal XZ)
    glm::vec3 planarMTV = glm::vec3(mtv.x, 0.0f, mtv.z);
    if (glm::length(planarMTV) > 0.0001f) {
        m_position += planarMTV;
    } else {
        m_position += mtv;
    }

    // Evaluar ángulo de incidencia del impacto
    glm::vec3 forward = getForwardVector();
    glm::vec3 mtvDir = (glm::length(planarMTV) > 0.0001f) ? glm::normalize(planarMTV) : glm::normalize(mtv);
    float dot = glm::dot(forward, mtvDir);

    if (std::abs(dot) < 0.45f) {
        // Deslizamiento rasante en paredes (Wall Sliding):
        // Preservar la inercia del kart reduciendo solo un 15% de velocidad en impactos laterales
        m_currentSpeed *= 0.85f;
    } else {
        // Impacto frontal o directo contra el obstaculo
        m_currentSpeed = -m_currentSpeed * reboundFactor;
    }
}

} // namespace physics
