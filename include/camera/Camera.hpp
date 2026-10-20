#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace camera {

enum class CameraMovement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

class Camera {
public:
    explicit Camera(glm::vec3 position = glm::vec3(0.0f, 2.5f, 6.0f),
                    glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f),
                    float yaw = -90.0f,
                    float pitch = -15.0f);

    // Matrices de transformación
    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix(float aspectRatio) const;

    // Control de movimiento interactivo FPS
    void processKeyboard(CameraMovement direction, float deltaTime);
    void processMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);
    void processMouseScroll(float yoffset);

    // Getters y Setters
    const glm::vec3& getPosition() const { return m_position; }
    const glm::vec3& getFront() const { return m_front; }
    const glm::vec3& getUp() const { return m_up; }
    const glm::vec3& getRight() const { return m_right; }

    float getYaw() const { return m_yaw; }
    float getPitch() const { return m_pitch; }
    float getFOV() const { return m_fov; }
    float getSpeed() const { return m_movementSpeed; }
    float getSensitivity() const { return m_mouseSensitivity; }

    void setPosition(const glm::vec3& pos) { m_position = pos; }
    void setSpeed(float speed) { m_movementSpeed = speed; }
    void setSensitivity(float sens) { m_mouseSensitivity = sens; }
    void setFOV(float fov) { m_fov = fov; }
    void lookAtTarget(const glm::vec3& target);

private:
    void updateCameraVectors();

    glm::vec3 m_position;
    glm::vec3 m_front{0.0f, 0.0f, -1.0f};
    glm::vec3 m_up{0.0f, 1.0f, 0.0f};
    glm::vec3 m_right{1.0f, 0.0f, 0.0f};
    glm::vec3 m_worldUp{0.0f, 1.0f, 0.0f};

    // Ángulos de Euler
    float m_yaw{-90.0f};
    float m_pitch{-15.0f};

    // Parámetros de cámara
    float m_movementSpeed{12.0f};
    float m_mouseSensitivity{0.12f};
    float m_fov{60.0f};
    float m_nearPlane{0.1f};
    float m_farPlane{1000.0f};
};

} // namespace camera
