#include "camera/Camera.hpp"
#include <algorithm>
#include <cmath>

namespace camera {

Camera::Camera(glm::vec3 position, glm::vec3 worldUp, float yaw, float pitch)
    : m_position(position)
    , m_worldUp(worldUp)
    , m_yaw(yaw)
    , m_pitch(pitch)
{
    updateCameraVectors();
}

glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(m_position, m_position + m_front, m_up);
}

glm::mat4 Camera::getProjectionMatrix(float aspectRatio) const {
    return glm::perspective(glm::radians(m_fov), aspectRatio, m_nearPlane, m_farPlane);
}

void Camera::updateCameraVectors() {
    float yawRad = glm::radians(m_yaw);
    float pitchRad = glm::radians(m_pitch);

    glm::vec3 newFront;
    newFront.x = std::cos(yawRad) * std::cos(pitchRad);
    newFront.y = std::sin(pitchRad);
    newFront.z = std::sin(yawRad) * std::cos(pitchRad);

    m_front = glm::normalize(newFront);
    m_right = glm::normalize(glm::cross(m_front, m_worldUp));
    m_up    = glm::normalize(glm::cross(m_right, m_front));
}

void Camera::processKeyboard(CameraMovement direction, float deltaTime) {
    float velocity = m_movementSpeed * deltaTime;

    // Movimiento en plano XZ para navegación FPS clásica
    glm::vec3 flatFront = glm::normalize(glm::vec3(m_front.x, 0.0f, m_front.z));
    if (glm::length(flatFront) < 0.001f) {
        flatFront = m_front;
    }

    if (direction == CameraMovement::FORWARD) {
        m_position += flatFront * velocity;
    }
    if (direction == CameraMovement::BACKWARD) {
        m_position -= flatFront * velocity;
    }
    if (direction == CameraMovement::LEFT) {
        m_position -= m_right * velocity;
    }
    if (direction == CameraMovement::RIGHT) {
        m_position += m_right * velocity;
    }
    if (direction == CameraMovement::UP) {
        m_position += m_worldUp * velocity;
    }
    if (direction == CameraMovement::DOWN) {
        m_position -= m_worldUp * velocity;
    }
}

void Camera::processMouseMovement(float xoffset, float yoffset, bool constrainPitch) {
    xoffset *= m_mouseSensitivity;
    yoffset *= m_mouseSensitivity;

    m_yaw   += xoffset;
    m_pitch += yoffset;

    if (constrainPitch) {
        m_pitch = std::clamp(m_pitch, -89.0f, 89.0f);
    }

    updateCameraVectors();
}

void Camera::processMouseScroll(float yoffset) {
    m_fov -= yoffset * 2.0f;
    m_fov = std::clamp(m_fov, 20.0f, 90.0f);
}

void Camera::lookAtTarget(const glm::vec3& target) {
    m_front = glm::normalize(target - m_position);
    m_pitch = glm::degrees(std::asin(m_front.y));
    m_yaw = glm::degrees(std::atan2(m_front.z, m_front.x));
    updateCameraVectors();
}

} // namespace camera
