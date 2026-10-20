#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>

namespace animation {

/**
 * @brief Controlador cinematico de animacion de ruedas en los ejes X (rodaje) e Y (viraje).
 */
class WheelAnimator {
public:
    explicit WheelAnimator(float wheelRadius = 0.22f, float maxSteerAngle = 28.0f);

    /**
     * @brief Actualiza la cinemática de las ruedas segun el desplazamiento y la direccion.
     * @param deltaTime Tiempo transcurrido en segundos.
     * @param speed Velocidad lineal actual del vehículo (m/s, con signo).
     * @param steerInput Entrada de viraje normalizada en [-1.0f, +1.0f].
     */
    void update(float deltaTime, float speed, float steerInput);

    [[nodiscard]] float getRollAngle() const { return m_rollAngle; }
    [[nodiscard]] float getSteerAngle() const { return m_currentSteerAngle; }
    [[nodiscard]] float getWheelRadius() const { return m_wheelRadius; }

    /**
     * @brief Retorna la rotacion local combinada para una rueda delantera Ry(steer) * Rx(roll).
     */
    [[nodiscard]] glm::mat4 getFrontWheelRotation() const {
        glm::mat4 rot = glm::rotate(glm::mat4(1.0f), glm::radians(m_currentSteerAngle), glm::vec3(0.0f, 1.0f, 0.0f));
        return glm::rotate(rot, glm::radians(m_rollAngle), glm::vec3(1.0f, 0.0f, 0.0f));
    }

    /**
     * @brief Retorna la rotacion local combinada para una rueda trasera Rx(roll).
     */
    [[nodiscard]] glm::mat4 getRearWheelRotation() const {
        return glm::rotate(glm::mat4(1.0f), glm::radians(m_rollAngle), glm::vec3(1.0f, 0.0f, 0.0f));
    }

    void reset();

private:
    float m_wheelRadius{0.22f};
    float m_maxSteerAngle{28.0f};
    float m_steerSpeed{12.0f}; // Velocidad de transicion angular

    float m_rollAngle{0.0f};        // Grados sobre eje X
    float m_currentSteerAngle{0.0f}; // Grados sobre eje Y
};

} // namespace animation
