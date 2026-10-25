#pragma once

#include <glm/glm.hpp>
#include "vfx/ParticleSystem.hpp"
#include "vfx/KartMotionState.hpp"

namespace Engine::Physics {
    class KartPhysics;
}

namespace vfx {

/**
 * @brief Emisor especializado para efectos visuales del kart (humo de escape y ráfaga de turbo).
 */
class KartParticleEmitter {
public:
    explicit KartParticleEmitter(size_t maxSmokeParticles = 250, size_t maxBoostParticles = 350);
    ~KartParticleEmitter() = default;

    // Inicialización de subsistemas y buffers GPU
    bool init();
    bool loadShaders(const std::string& vertPath = "shaders/particle.vert",
                     const std::string& fragPath = "shaders/particle.frag");

    // Actualización desacoplada mediante Value Object plano
    void update(float deltaTime, const KartMotionState& motion);

    // Adaptador de compatibilidad para KartPhysics
    void update(float deltaTime, const Engine::Physics::KartPhysics& physics, bool isAccelerating);

    // Renderizado en dos fases: humo translúcido y ráfaga aditiva luminosa
    void render(const glm::mat4& view, const glm::mat4& projection);
    void render(const renderer::Shader& shader, const glm::mat4& view, const glm::mat4& projection);

    // Disparo explícito de ráfaga de turbo (al cruzar BoosterPad)
    void triggerBoostBurst(const glm::vec3& kartPos, float kartYawDeg, float intensity = 1.0f);

    // Cálculo de coordenadas mundiales de los tubos de escape
    [[nodiscard]] glm::vec3 getExhaustWorldPosition(const glm::vec3& kartPos, float kartYawDeg, bool isLeft) const;

    // Acceso a los sistemas de partículas internos
    ParticleSystem& getSmokeSystem() { return m_smokeSystem; }
    const ParticleSystem& getSmokeSystem() const { return m_smokeSystem; }

    ParticleSystem& getBoostSystem() { return m_boostSystem; }
    const ParticleSystem& getBoostSystem() const { return m_boostSystem; }

    // Configuración de offsets locales de escape
    void setExhaustOffsets(const glm::vec3& left, const glm::vec3& right) {
        m_exhaustOffsetLeft = left;
        m_exhaustOffsetRight = right;
    }

    // Modo subacuático: sustituye el humo por estelas de burbujas ascendentes
    void setUnderwater(bool underwater) { m_isUnderwater = underwater; }
    [[nodiscard]] bool isUnderwater() const { return m_isUnderwater; }

private:
    ParticleSystem m_smokeSystem;
    ParticleSystem m_boostSystem;

    glm::vec3 m_exhaustOffsetLeft{-0.22f, 0.22f, -0.70f};
    glm::vec3 m_exhaustOffsetRight{0.22f, 0.22f, -0.70f};

    float m_smokeTimer{0.0f};
    float m_boostTrailTimer{0.0f};
    bool m_wasBoosting{false};
    bool m_toggleExhaust{false};
    bool m_isUnderwater{false};
};

} // namespace vfx
