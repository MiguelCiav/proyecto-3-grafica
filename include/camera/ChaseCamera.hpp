#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace camera {

/**
 * @brief Camara en tercera persona (Chase Cam) que sigue al kart suavemente.
 */
class ChaseCamera {
public:
    explicit ChaseCamera(float distance = 5.2f, float height = 2.4f, float lookAhead = 1.2f);

    void update(float deltaTime, const glm::vec3& targetPos, const glm::vec3& targetForward);
    void reset(const glm::vec3& targetPos, const glm::vec3& targetForward);

    [[nodiscard]] glm::mat4 getViewMatrix() const;
    [[nodiscard]] glm::mat4 getProjectionMatrix(float aspectRatio) const;

    [[nodiscard]] const glm::vec3& getPosition() const { return m_currentPos; }
    [[nodiscard]] const glm::vec3& getTarget() const { return m_currentTarget; }
    [[nodiscard]] float getFOV() const { return m_fov; }

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
