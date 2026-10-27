#pragma once

#include "camera/ICamera.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace physics {
    class CollisionSystem;
}

namespace Engine::Physics {
    class CollisionSystem;
}

namespace camera {

enum class CameraMovement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

class Camera : public ICamera {
public:
    explicit Camera(glm::vec3 position = glm::vec3(0.0f, 2.5f, 6.0f),
                    glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f),
                    float yaw = -90.0f,
                    float pitch = -15.0f);

    // Matrices de transformacion (ICamera)
    [[nodiscard]] glm::mat4 getViewMatrix() const override;
    [[nodiscard]] glm::mat4 getProjectionMatrix(float aspectRatio) const override;

    // Control de movimiento interactivo FPS
    void processKeyboard(CameraMovement direction, float deltaTime);
    void processKeyboardWithCollision(CameraMovement direction, float deltaTime, const physics::CollisionSystem& collisionSystem);
    void processKeyboardWithCollision(CameraMovement direction, float deltaTime, const Engine::Physics::CollisionSystem& collisionSystem);
    void processMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);
    void processMouseScroll(float yoffset);

    // Resolucion de colision contra obstaculos (PDF Requisito Minimo)
    void resolveCollision(const physics::CollisionSystem& collisionSystem);
    void resolveCollision(const Engine::Physics::CollisionSystem& collisionSystem);

    // Getters polimorficos (ICamera)
    [[nodiscard]] const glm::vec3& getPosition() const override { return m_position; }
    [[nodiscard]] glm::vec3 getForwardVector() const override { return m_front; }
    [[nodiscard]] float getFOV() const override { return m_fov; }

    // Getters y Setters especificos FPS
    const glm::vec3& getFront() const { return m_front; }
    const glm::vec3& getUp() const { return m_up; }
    const glm::vec3& getRight() const { return m_right; }

    float getYaw() const { return m_yaw; }
    float getPitch() const { return m_pitch; }
    float getSpeed() const { return m_movementSpeed; }
    float getSensitivity() const { return m_mouseSensitivity; }
    float getCollisionRadius() const { return m_collisionRadius; }
    float getMinY() const { return m_minY; }
    float getMaxY() const { return m_maxY; }

    void setPosition(const glm::vec3& pos) { m_position = pos; }
    void setSpeed(float speed) { m_movementSpeed = speed; }
    void setSensitivity(float sens) { m_mouseSensitivity = sens; }
    void setFOV(float fov) { m_fov = fov; }
    void setCollisionRadius(float radius) { m_collisionRadius = radius; }
    void setCollisionEnabled(bool enabled) { m_collisionEnabled = enabled; }
    bool isCollisionEnabled() const { return m_collisionEnabled; }
    void setMinY(float minY) { m_minY = minY; }
    void setMaxY(float maxY) { m_maxY = maxY; }
    void setVerticalBounds(float minY, float maxY) { m_minY = minY; m_maxY = maxY; }

    void lookAtTarget(const glm::vec3& target);

private:
    void updateCameraVectors();

    glm::vec3 m_position;
    glm::vec3 m_front{0.0f, 0.0f, -1.0f};
    glm::vec3 m_up{0.0f, 1.0f, 0.0f};
    glm::vec3 m_right{1.0f, 0.0f, 0.0f};
    glm::vec3 m_worldUp{0.0f, 1.0f, 0.0f};

    // Angulos de Euler
    float m_yaw{-90.0f};
    float m_pitch{-15.0f};

    // Parametros de camara
    float m_movementSpeed{12.0f};
    float m_mouseSensitivity{0.12f};
    float m_fov{60.0f};
    float m_nearPlane{0.1f};
    float m_farPlane{1000.0f};

    // Restriccion de colision (Requisito UCV)
    float m_collisionRadius{0.45f};
    bool m_collisionEnabled{true};
    float m_minY{0.5f};
    float m_maxY{80.0f};
};

} // namespace camera
