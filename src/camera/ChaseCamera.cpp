#include "camera/ChaseCamera.hpp"
#include <algorithm>

namespace camera {

ChaseCamera::ChaseCamera(float distance, float height, float lookAhead)
    : m_distance(distance), m_height(height), m_lookAhead(lookAhead) {}

void ChaseCamera::reset(const glm::vec3& targetPos, const glm::vec3& targetForward) {
    glm::vec3 normForward = glm::length(targetForward) > 0.001f ? glm::normalize(targetForward) : glm::vec3(0.0f, 0.0f, 1.0f);
    m_currentPos = targetPos - normForward * m_distance + glm::vec3(0.0f, m_height, 0.0f);
    m_currentTarget = targetPos + glm::vec3(0.0f, 0.45f, 0.0f) + normForward * m_lookAhead;
}

void ChaseCamera::update(float deltaTime, const glm::vec3& targetPos, const glm::vec3& targetForward) {
    glm::vec3 normForward = glm::length(targetForward) > 0.001f ? glm::normalize(targetForward) : glm::vec3(0.0f, 0.0f, 1.0f);

    glm::vec3 desiredPos = targetPos - normForward * m_distance + glm::vec3(0.0f, m_height, 0.0f);
    glm::vec3 desiredTarget = targetPos + glm::vec3(0.0f, 0.45f, 0.0f) + normForward * m_lookAhead;

    // Interpolacion exponencial suave para evitar saltos o camara rigida
    float blend = 1.0f - std::exp(-m_smoothSpeed * deltaTime);
    m_currentPos = glm::mix(m_currentPos, desiredPos, blend);
    m_currentTarget = glm::mix(m_currentTarget, desiredTarget, blend);
}

glm::mat4 ChaseCamera::getViewMatrix() const {
    return glm::lookAt(m_currentPos, m_currentTarget, m_up);
}

glm::mat4 ChaseCamera::getProjectionMatrix(float aspectRatio) const {
    return glm::perspective(glm::radians(m_fov), aspectRatio, m_nearPlane, m_farPlane);
}

} // namespace camera
