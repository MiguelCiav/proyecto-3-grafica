#pragma once

#include <glm/glm.hpp>

namespace vfx {

struct Particle {
    glm::vec3 position{0.0f};
    glm::vec3 velocity{0.0f};
    glm::vec4 color{1.0f};
    glm::vec4 baseColor{1.0f};
    float size{1.0f};
    float baseSize{1.0f};
    float life{1.0f};
    float remainingLife{0.0f};

    bool isAlive() const {
        return remainingLife > 0.0f;
    }
};

} // namespace vfx
