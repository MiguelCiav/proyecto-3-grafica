#pragma once

#include <glm/glm.hpp>

namespace vfx {

/**
 * @brief Estado cinemático plano requerido para la emisión de efectos visuales (VFX).
 * Desacopla los emisores de partículas de la implementación física del kart.
 */
struct KartMotionState {
    glm::vec3 position{0.0f};
    float yawDegrees{0.0f};
    float speed{0.0f};
    bool isAccelerating{false};
    bool isBoosting{false};
};

} // namespace vfx
