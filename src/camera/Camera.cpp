#include "camera/Camera.hpp"
#include "physics/CollisionSystem.hpp"
#include "physics/CollisionSolver.hpp"
#include <algorithm>

namespace camera {

Camera::Camera(glm::vec3 position, glm::vec3 worldUp, float yaw, float pitch)
    : m_position(position),
      m_worldUp(worldUp),
      m_yaw(yaw),
      m_pitch(pitch) {
    updateCameraVectors();
}

glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(m_position, m_position + m_front, m_up);
}

glm::mat4 Camera::getProjectionMatrix(float aspectRatio) const {
    return glm::perspective(glm::radians(m_fov), aspectRatio, m_nearPlane, m_farPlane);
}

void Camera::processKeyboard(CameraMovement direction, float deltaTime) {
    float velocity = m_movementSpeed * deltaTime;

    if (direction == CameraMovement::FORWARD)  m_position += m_front * velocity;
    if (direction == CameraMovement::BACKWARD) m_position -= m_front * velocity;
    if (direction == CameraMovement::LEFT)     m_position -= m_right * velocity;
    if (direction == CameraMovement::RIGHT)    m_position += m_right * velocity;
    if (direction == CameraMovement::UP)       m_position += m_worldUp * velocity;
    if (direction == CameraMovement::DOWN)     m_position -= m_worldUp * velocity;
}

void Camera::processKeyboardWithCollision(CameraMovement direction, float deltaTime, const physics::CollisionSystem& collisionSystem) {
    processKeyboard(direction, deltaTime);
    if (m_collisionEnabled) {
        resolveCollision(collisionSystem);
    }
}

void Camera::processKeyboardWithCollision(CameraMovement direction, float deltaTime, const Engine::Physics::CollisionSystem& collisionSystem) {
    processKeyboardWithCollision(direction, deltaTime, static_cast<const physics::CollisionSystem&>(collisionSystem));
}

void Camera::resolveCollision(const physics::CollisionSystem& collisionSystem) {
    m_position = physics::CollisionSolver::resolveCameraPosition(m_position, m_collisionRadius, collisionSystem, m_minY, m_maxY);
}

void Camera::resolveCollision(const Engine::Physics::CollisionSystem& collisionSystem) {
    resolveCollision(static_cast<const physics::CollisionSystem&>(collisionSystem));
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
    m_fov -= yoffset;
    m_fov = std::clamp(m_fov, 15.0f, 90.0f);
}

void Camera::lookAtTarget(const glm::vec3& target) {
    m_front = glm::normalize(target - m_position);
    m_pitch = glm::degrees(std::asin(std::clamp(m_front.y, -1.0f, 1.0f)));
    m_yaw = glm::degrees(std::atan2(m_front.z, m_front.x));
    updateCameraVectors();
}

void Camera::updateCameraVectors() {
    glm::vec3 front;
    front.x = std::cos(glm::radians(m_yaw)) * std::cos(glm::radians(m_pitch));
    front.y = std::sin(glm::radians(m_pitch));
    front.z = std::sin(glm::radians(m_yaw)) * std::cos(glm::radians(m_pitch));
    m_front = glm::normalize(front);

    m_right = glm::normalize(glm::cross(m_front, m_worldUp));
    m_up    = glm::normalize(glm::cross(m_right, m_front));
}

} // namespace camera
