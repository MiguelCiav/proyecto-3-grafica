#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace scene {

/**
 * @brief Representacion de una transformacion local 3D (Posicion, Rotacion, Escala).
 */
class Transform {
public:
    Transform() = default;
    explicit Transform(const glm::vec3& pos, const glm::vec3& rotDeg = glm::vec3(0.0f), const glm::vec3& scl = glm::vec3(1.0f))
        : position(pos), rotationEuler(rotDeg), scale(scl) {}

    glm::vec3 position{0.0f};
    glm::vec3 rotationEuler{0.0f}; // Grados: pitch (X), yaw (Y), roll (Z)
    glm::vec3 scale{1.0f};

    /**
     * @brief Retorna la matriz de transformacion local M = T * R * S.
     * Orden de rotacion: Yaw (Y) -> Pitch (X) -> Roll (Z) para evitar gimbal lock comun.
     */
    [[nodiscard]] glm::mat4 getLocalMatrix() const {
        glm::mat4 mat = glm::translate(glm::mat4(1.0f), position);
        if (rotationEuler.y != 0.0f) {
            mat = glm::rotate(mat, glm::radians(rotationEuler.y), glm::vec3(0.0f, 1.0f, 0.0f));
        }
        if (rotationEuler.x != 0.0f) {
            mat = glm::rotate(mat, glm::radians(rotationEuler.x), glm::vec3(1.0f, 0.0f, 0.0f));
        }
        if (rotationEuler.z != 0.0f) {
            mat = glm::rotate(mat, glm::radians(rotationEuler.z), glm::vec3(0.0f, 0.0f, 1.0f));
        }
        if (scale != glm::vec3(1.0f)) {
            mat = glm::scale(mat, scale);
        }
        return mat;
    }

    void translate(const glm::vec3& delta) { position += delta; }
    void rotateX(float deg) { rotationEuler.x += deg; }
    void rotateY(float deg) { rotationEuler.y += deg; }
    void rotateZ(float deg) { rotationEuler.z += deg; }
};

} // namespace scene
