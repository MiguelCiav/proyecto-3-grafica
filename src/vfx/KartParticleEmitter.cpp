#include "vfx/KartParticleEmitter.hpp"
#include <glm/gtc/constants.hpp>
#include <random>
#include <cmath>
#include <algorithm>

namespace vfx {

static float getRandomFloat(float minVal, float maxVal) {
    static std::mt19937 gen(1337);
    std::uniform_real_distribution<float> dis(minVal, maxVal);
    return dis(gen);
}

KartParticleEmitter::KartParticleEmitter(size_t maxSmokeParticles, size_t maxBoostParticles)
    : m_smokeSystem(maxSmokeParticles),
      m_boostSystem(maxBoostParticles) {
    // Configuración del sistema de humo: flotabilidad ascendente y expansión
    m_smokeSystem.setGravity(glm::vec3(0.0f, 0.6f, 0.0f));
    m_smokeSystem.setDamping(1.4f);
    m_smokeSystem.setSizeGrowth(0.55f);
    m_smokeSystem.setAdditive(false);

    // Configuración del sistema de turbo: ráfagas hiperactivas aditivas y chispas
    m_boostSystem.setGravity(glm::vec3(0.0f, -2.8f, 0.0f));
    m_boostSystem.setDamping(0.7f);
    m_boostSystem.setSizeGrowth(0.05f);
    m_boostSystem.setAdditive(true);
}

bool KartParticleEmitter::init() {
    bool okSmoke = m_smokeSystem.init();
    bool okBoost = m_boostSystem.init();
    return okSmoke && okBoost;
}

bool KartParticleEmitter::loadShaders(const std::string& vertPath, const std::string& fragPath) {
    bool okSmoke = m_smokeSystem.loadShader(vertPath, fragPath);
    bool okBoost = m_boostSystem.loadShader(vertPath, fragPath);
    return okSmoke && okBoost;
}

glm::vec3 KartParticleEmitter::getExhaustWorldPosition(const glm::vec3& kartPos, float kartYawDeg, bool isLeft) const {
    float rad = glm::radians(kartYawDeg);
    glm::vec3 forward(std::sin(rad), 0.0f, std::cos(rad));
    glm::vec3 right(std::cos(rad), 0.0f, -std::sin(rad));
    glm::vec3 up(0.0f, 1.0f, 0.0f);

    const glm::vec3& offset = isLeft ? m_exhaustOffsetLeft : m_exhaustOffsetRight;
    return kartPos + right * offset.x + up * offset.y + forward * offset.z;
}

void KartParticleEmitter::triggerBoostBurst(const glm::vec3& kartPos, float kartYawDeg, float intensity) {
    float rad = glm::radians(kartYawDeg);
    glm::vec3 forward(std::sin(rad), 0.0f, std::cos(rad));
    glm::vec3 right(std::cos(rad), 0.0f, -std::sin(rad));
    glm::vec3 up(0.0f, 1.0f, 0.0f);

    int count = static_cast<int>(40 * intensity);
    for (int i = 0; i < count; ++i) {
        bool isLeft = (i % 2 == 0);
        glm::vec3 origin = getExhaustWorldPosition(kartPos, kartYawDeg, isLeft);

        // Proyección cónica de chispas hacia atrás
        float spreadX = getRandomFloat(-3.2f, 3.2f);
        float spreadY = getRandomFloat(-0.8f, 2.5f);
        float speedBack = getRandomFloat(16.0f, 26.0f);

        glm::vec3 vel = -forward * speedBack + right * spreadX + up * spreadY;

        // Alternancia de colores neón cian y naranja eléctrico
        glm::vec4 color;
        if (i % 3 == 0) {
            color = glm::vec4(0.1f, 0.85f, 1.0f, 0.95f); // Azul cian neón
        } else if (i % 3 == 1) {
            color = glm::vec4(1.0f, 0.6f, 0.05f, 0.95f);  // Naranja llama
        } else {
            color = glm::vec4(1.0f, 0.95f, 0.4f, 1.0f);   // Núcleo blanco/amarillo incandescente
        }

        float size = getRandomFloat(0.18f, 0.35f);
        float life = getRandomFloat(0.35f, 0.65f);

        m_boostSystem.emit(origin, vel, color, size, life);
    }
}

void KartParticleEmitter::update(float deltaTime, const Engine::Physics::KartPhysics& physics, bool isAccelerating) {
    if (deltaTime <= 0.0f) return;

    const glm::vec3 kartPos = physics.getPosition();
    const float kartYaw = physics.getYaw();
    const float speed = physics.getSpeed();
    const bool isBoosting = physics.isBoosting();

    float rad = glm::radians(kartYaw);
    glm::vec3 forward(std::sin(rad), 0.0f, std::cos(rad));
    glm::vec3 right(std::cos(rad), 0.0f, -std::sin(rad));
    glm::vec3 up(0.0f, 1.0f, 0.0f);

    // -------------------------------------------------------------------------
    // 1. Detección y emisión de Boost Burst y estela continua de turbo
    // -------------------------------------------------------------------------
    if (isBoosting && !m_wasBoosting) {
        // Disparo explosivo de inicio de turbo
        triggerBoostBurst(kartPos, kartYaw, 1.0f);
    }
    m_wasBoosting = isBoosting;

    if (isBoosting) {
        m_boostTrailTimer += deltaTime;
        const float trailCadence = 0.02f; // Emisión densa a alta frecuencia
        while (m_boostTrailTimer >= trailCadence) {
            m_boostTrailTimer -= trailCadence;
            m_toggleExhaust = !m_toggleExhaust;

            glm::vec3 origin = getExhaustWorldPosition(kartPos, kartYaw, m_toggleExhaust);
            float spreadX = getRandomFloat(-0.8f, 0.8f);
            float spreadY = getRandomFloat(-0.4f, 0.8f);
            float speedBack = getRandomFloat(12.0f, 18.0f);

            glm::vec3 vel = -forward * speedBack + right * spreadX + up * spreadY;
            glm::vec4 color = (getRandomFloat(0.0f, 1.0f) > 0.4f)
                ? glm::vec4(0.15f, 0.8f, 1.0f, 0.9f)
                : glm::vec4(1.0f, 0.5f, 0.1f, 0.9f);

            m_boostSystem.emit(origin, vel, color, getRandomFloat(0.2f, 0.32f), getRandomFloat(0.25f, 0.45f));
        }
    } else {
        m_boostTrailTimer = 0.0f;
    }

    // -------------------------------------------------------------------------
    // 2. Emisión reactiva de humo de escape según aceleración y ralentí
    // -------------------------------------------------------------------------
    m_smokeTimer += deltaTime;

    float smokeCadence = 0.35f; // Ralentí por defecto
    if (isAccelerating) {
        // Cadencia rápida proporcional a la velocidad y aceleración
        float speedFactor = std::clamp(std::abs(speed) / 20.0f, 0.0f, 1.0f);
        smokeCadence = std::max(0.035f, 0.08f - speedFactor * 0.045f);
    }

    while (m_smokeTimer >= smokeCadence) {
        m_smokeTimer -= smokeCadence;
        m_toggleExhaust = !m_toggleExhaust;

        glm::vec3 origin = getExhaustWorldPosition(kartPos, kartYaw, m_toggleExhaust);

        glm::vec3 vel;
        glm::vec4 color;
        float size;
        float life;

        if (isAccelerating) {
            float ejectSpeed = std::abs(speed) * 0.35f + getRandomFloat(1.2f, 2.5f);
            float spreadX = getRandomFloat(-0.35f, 0.35f);
            float spreadZ = getRandomFloat(-0.35f, 0.35f);
            vel = -forward * ejectSpeed + right * spreadX + forward * spreadZ + up * getRandomFloat(0.6f, 1.4f);

            float shade = getRandomFloat(0.75f, 0.90f);
            color = glm::vec4(shade, shade, shade, getRandomFloat(0.45f, 0.65f));
            size = getRandomFloat(0.16f, 0.24f);
            life = getRandomFloat(0.65f, 1.0f);
        } else {
            // Humo tenue y lento en reposo
            vel = up * getRandomFloat(0.4f, 0.8f) + right * getRandomFloat(-0.1f, 0.1f);
            float shade = getRandomFloat(0.85f, 0.95f);
            color = glm::vec4(shade, shade, shade, getRandomFloat(0.15f, 0.28f));
            size = getRandomFloat(0.12f, 0.18f);
            life = getRandomFloat(0.8f, 1.2f);
        }

        m_smokeSystem.emit(origin, vel, color, size, life);
    }

    // -------------------------------------------------------------------------
    // 3. Actualización física de ambos subsistemas en el espacio del mundo
    // -------------------------------------------------------------------------
    m_smokeSystem.update(deltaTime);
    m_boostSystem.update(deltaTime);
}

void KartParticleEmitter::render(const glm::mat4& view, const glm::mat4& projection) {
    m_smokeSystem.render(view, projection);
    m_boostSystem.render(view, projection);
}

void KartParticleEmitter::render(const renderer::Shader& shader, const glm::mat4& view, const glm::mat4& projection) {
    m_smokeSystem.render(shader, view, projection);
    m_boostSystem.render(shader, view, projection);
}

} // namespace vfx
