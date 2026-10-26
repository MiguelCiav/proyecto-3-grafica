#pragma once

#include "camera/ICamera.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace camera {

/**
 * @brief Camara en tercera persona (Chase Cam) que sigue al kart suavemente.
 */
class ChaseCamera : public ICamera {
public:
    explicit ChaseCamera(float distance = 2.2f, float height = 0.95f, float lookAhead = 0.85f);

    void update(float deltaTime, const glm::vec3& targetPos, const glm::vec3& targetForward);
    void reset(const glm::vec3& targetPos, const glm::vec3& targetForward);

    [[nodiscard]] glm::mat4 getViewMatrix() const override;
    [[nodiscard]] glm::mat4 getProjectionMatrix(float aspectRatio) const override;

    [[nodiscard]] const glm::vec3& getPosition() const override { return m_currentPos; }
    [[nodiscard]] glm::vec3 getForwardVector() const override {
        glm::vec3 dir = m_currentTarget - m_currentPos;
        return (glm::length(dir) > 0.0001f) ? glm::normalize(dir) : glm::vec3(0.0f, 0.0f, 1.0f);
    }
    [[nodiscard]] const glm::vec3& getTarget() const { return m_currentTarget; }
    [[nodiscard]] float getFOV() const override { return m_fov; }

    void setFOV(float fov) { m_fov = fov; }
    void setDistance(float dist) { m_distance = dist; }
    void setHeight(float h) { m_height = h; }

private:
    float m_distance;
    float m_height;
    float m_lookAhead;
    float m_smoothSpeed{9.0f};

    glm::vec3 m_currentPos{0.0f, 3.0f, -5.0f};
    glm::vec3 m_currentTarget{0.0f, 1.0f, 0.0f};
    glm::vec3 m_up{0.0f, 1.0f, 0.0f};

    float m_fov{65.0f};
    float m_nearPlane{0.1f};
    float m_farPlane{1000.0f};
};

} // namespace camera
