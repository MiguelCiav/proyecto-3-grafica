#pragma once

#include <glm/glm.hpp>

namespace camera {

/**
 * @brief Interfaz base polimórfica para cámaras del motor (FPS, Chase, Cinematográfica).
 * Permite que los shaders, minimapas y pases de renderizado operen contra cualquier cámara
 * sin bifurcaciones ternarias en el cliente.
 */
class ICamera {
public:
    virtual ~ICamera() = default;

    [[nodiscard]] virtual glm::mat4 getViewMatrix() const = 0;
    [[nodiscard]] virtual glm::mat4 getProjectionMatrix(float aspectRatio) const = 0;
    [[nodiscard]] virtual const glm::vec3& getPosition() const = 0;
    [[nodiscard]] virtual glm::vec3 getForwardVector() const = 0;
    [[nodiscard]] virtual float getFOV() const = 0;
};

} // namespace camera
