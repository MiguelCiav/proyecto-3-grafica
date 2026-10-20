#include "animation/WheelAnimator.hpp"

namespace animation {

WheelAnimator::WheelAnimator(float wheelRadius, float maxSteerAngle)
    : m_wheelRadius(wheelRadius),
      m_maxSteerAngle(maxSteerAngle) {}

void WheelAnimator::reset() {
    m_rollAngle = 0.0f;
    m_currentSteerAngle = 0.0f;
}

void WheelAnimator::update(float deltaTime, float speed, float steerInput) {
    if (deltaTime <= 0.0f) return;

    // 1. Rodamiento en eje X: proporcional al desplazamiento lineal Delta_x = v * dt / r
    if (m_wheelRadius > 0.001f) {
        float deltaRoll = glm::degrees((speed * deltaTime) / m_wheelRadius);
        m_rollAngle += deltaRoll;

        // Mantener el angulo acotado dentro de [-360, 360]
        while (m_rollAngle > 360.0f)  m_rollAngle -= 360.0f;
        while (m_rollAngle < -360.0f) m_rollAngle += 360.0f;
    }

    // 2. Viraje en eje Y (volante / ruedas delanteras):
    // steerInput: -1.0 (izq), +1.0 (der). El angulo de direccion se suaviza hacia el objetivo
    float targetSteer = std::clamp(steerInput, -1.0f, 1.0f) * m_maxSteerAngle;
    float blend = 1.0f - std::exp(-m_steerSpeed * deltaTime);
    m_currentSteerAngle = glm::mix(m_currentSteerAngle, targetSteer, blend);
}

} // namespace animation
