#pragma once

#include <vector>
#include <string>
#include <glm/glm.hpp>
#include "vfx/Particle.hpp"
#include "renderer/Shader.hpp"

namespace vfx {

struct ParticleInstanceData {
    glm::vec3 position{0.0f};
    float size{1.0f};
    glm::vec4 color{1.0f};
};

class ParticleSystem {
public:
    explicit ParticleSystem(size_t maxParticles = 500);
    ~ParticleSystem();

    ParticleSystem(const ParticleSystem&) = delete;
    ParticleSystem& operator=(const ParticleSystem&) = delete;

    ParticleSystem(ParticleSystem&& other) noexcept;
    ParticleSystem& operator=(ParticleSystem&& other) noexcept;

    // Inicialización de buffers GPU (VAO, VBO, Instance VBO, EBO)
    bool init();

    // Carga de shader por defecto
    bool loadShader(const std::string& vertPath = "shaders/particle.vert",
                    const std::string& fragPath = "shaders/particle.frag");

    // Actualización de física, ciclo de vida y desvanecimiento progresivo
    void update(float deltaTime);

    // Renderizado usando el shader interno o un shader provisto externamente
    void render(const glm::mat4& view, const glm::mat4& projection);
    void render(const renderer::Shader& shader, const glm::mat4& view, const glm::mat4& projection);

    // Emisión de partículas en el pool prealocado
    bool emit(const glm::vec3& position,
              const glm::vec3& velocity,
              const glm::vec4& color,
              float size,
              float life);

    bool emit(const Particle& particle);

    // Configuración física
    void setGravity(const glm::vec3& gravity) { m_gravity = gravity; }
    const glm::vec3& getGravity() const { return m_gravity; }

    void setDamping(float damping) { m_damping = damping; }
    float getDamping() const { return m_damping; }

    void setAdditive(bool additive) { m_additive = additive; }
    bool isAdditive() const { return m_additive; }

    void setSizeGrowth(float growthRate) { m_sizeGrowth = growthRate; }
    float getSizeGrowth() const { return m_sizeGrowth; }

    // Control y estadísticas
    void clear();
    size_t getMaxParticles() const { return m_maxParticles; }
    size_t getActiveParticleCount() const { return m_activeCount; }
    const std::vector<Particle>& getParticles() const { return m_pool; }
    bool isInitialized() const { return m_initialized; }

private:
    void initBuffers();
    void releaseBuffers();

    size_t m_maxParticles{500};
    size_t m_activeCount{0};

    std::vector<Particle> m_pool;
    std::vector<ParticleInstanceData> m_instanceData;

    glm::vec3 m_gravity{0.0f, 0.0f, 0.0f};
    float m_damping{0.0f};
    float m_sizeGrowth{0.0f};
    bool m_additive{false};

    renderer::Shader m_shader;
    bool m_shaderLoaded{false};

    unsigned int m_quadVAO{0};
    unsigned int m_quadVBO{0};
    unsigned int m_quadEBO{0};
    unsigned int m_instanceVBO{0};
    bool m_initialized{false};
};

} // namespace vfx
