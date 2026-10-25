#include "vfx/ParticleSystem.hpp"
#include <glad/glad.h>
#include <algorithm>
#include <iostream>

namespace vfx {

ParticleSystem::ParticleSystem(size_t maxParticles)
    : m_maxParticles(maxParticles),
      m_activeCount(0),
      m_gravity(0.0f),
      m_damping(0.0f),
      m_sizeGrowth(0.0f),
      m_additive(false),
      m_shaderLoaded(false),
      m_quadVAO(0),
      m_quadVBO(0),
      m_quadEBO(0),
      m_instanceVBO(0),
      m_initialized(false) {
    m_pool.resize(m_maxParticles);
    m_instanceData.resize(m_maxParticles);
}

ParticleSystem::~ParticleSystem() {
    releaseBuffers();
}

ParticleSystem::ParticleSystem(ParticleSystem&& other) noexcept
    : m_maxParticles(other.m_maxParticles),
      m_activeCount(other.m_activeCount),
      m_pool(std::move(other.m_pool)),
      m_instanceData(std::move(other.m_instanceData)),
      m_gravity(other.m_gravity),
      m_damping(other.m_damping),
      m_sizeGrowth(other.m_sizeGrowth),
      m_additive(other.m_additive),
      m_shader(std::move(other.m_shader)),
      m_shaderLoaded(other.m_shaderLoaded),
      m_quadVAO(other.m_quadVAO),
      m_quadVBO(other.m_quadVBO),
      m_quadEBO(other.m_quadEBO),
      m_instanceVBO(other.m_instanceVBO),
      m_initialized(other.m_initialized) {
    other.m_activeCount = 0;
    other.m_quadVAO = 0;
    other.m_quadVBO = 0;
    other.m_quadEBO = 0;
    other.m_instanceVBO = 0;
    other.m_initialized = false;
    other.m_shaderLoaded = false;
}

ParticleSystem& ParticleSystem::operator=(ParticleSystem&& other) noexcept {
    if (this != &other) {
        releaseBuffers();

        m_maxParticles = other.m_maxParticles;
        m_activeCount = other.m_activeCount;
        m_pool = std::move(other.m_pool);
        m_instanceData = std::move(other.m_instanceData);
        m_gravity = other.m_gravity;
        m_damping = other.m_damping;
        m_sizeGrowth = other.m_sizeGrowth;
        m_additive = other.m_additive;
        m_shader = std::move(other.m_shader);
        m_shaderLoaded = other.m_shaderLoaded;
        m_quadVAO = other.m_quadVAO;
        m_quadVBO = other.m_quadVBO;
        m_quadEBO = other.m_quadEBO;
        m_instanceVBO = other.m_instanceVBO;
        m_initialized = other.m_initialized;

        other.m_activeCount = 0;
        other.m_quadVAO = 0;
        other.m_quadVBO = 0;
        other.m_quadEBO = 0;
        other.m_instanceVBO = 0;
        other.m_initialized = false;
        other.m_shaderLoaded = false;
    }
    return *this;
}

bool ParticleSystem::init() {
    initBuffers();
    return m_initialized;
}

void ParticleSystem::initBuffers() {
    if (m_initialized) return;

    static const float quadVertices[] = {
        // Posicion (x, y)   Coords UV (u, v)
        -0.5f, -0.5f,        0.0f, 0.0f,
         0.5f, -0.5f,        1.0f, 0.0f,
         0.5f,  0.5f,        1.0f, 1.0f,
        -0.5f,  0.5f,        0.0f, 1.0f
    };

    static const unsigned int quadIndices[] = {
        0, 1, 2,
        2, 3, 0
    };

    glGenVertexArrays(1, &m_quadVAO);
    glBindVertexArray(m_quadVAO);

    glGenBuffers(1, &m_quadVBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glGenBuffers(1, &m_quadEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_quadEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), quadIndices, GL_STATIC_DRAW);

    // Atributo 0: Posicion del vertice en el cuadrilatero [-0.5, 0.5]
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void*>(0));

    // Atributo 1: Coordenadas de textura UV [0, 1]
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void*>(2 * sizeof(float)));

    // Buffer instanciado para todas las particulas activas del pool
    glGenBuffers(1, &m_instanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, m_maxParticles * sizeof(ParticleInstanceData), nullptr, GL_DYNAMIC_DRAW);

    // Atributo 2: Posicion 3D de la particula en el espacio del mundo
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(ParticleInstanceData),
                          reinterpret_cast<void*>(offsetof(ParticleInstanceData, position)));
    glVertexAttribDivisor(2, 1);

    // Atributo 3: Escala / tamano de la particula
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(ParticleInstanceData),
                          reinterpret_cast<void*>(offsetof(ParticleInstanceData, size)));
    glVertexAttribDivisor(3, 1);

    // Atributo 4: Color RGBA (incluyendo canal alfa)
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleInstanceData),
                          reinterpret_cast<void*>(offsetof(ParticleInstanceData, color)));
    glVertexAttribDivisor(4, 1);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    m_initialized = true;
}

void ParticleSystem::releaseBuffers() {
    if (m_instanceVBO != 0) {
        glDeleteBuffers(1, &m_instanceVBO);
        m_instanceVBO = 0;
    }
    if (m_quadEBO != 0) {
        glDeleteBuffers(1, &m_quadEBO);
        m_quadEBO = 0;
    }
    if (m_quadVBO != 0) {
        glDeleteBuffers(1, &m_quadVBO);
        m_quadVBO = 0;
    }
    if (m_quadVAO != 0) {
        glDeleteVertexArrays(1, &m_quadVAO);
        m_quadVAO = 0;
    }
    m_initialized = false;
}

bool ParticleSystem::loadShader(const std::string& vertPath, const std::string& fragPath) {
    if (m_shader.loadFromFiles(vertPath, fragPath)) {
        m_shaderLoaded = true;
        return true;
    }
    m_shaderLoaded = false;
    return false;
}

void ParticleSystem::update(float deltaTime) {
    if (deltaTime <= 0.0f) return;

    for (size_t i = 0; i < m_activeCount; ) {
        auto& p = m_pool[i];
        p.remainingLife -= deltaTime;

        if (p.remainingLife <= 0.0f) {
            // Reciclado en O(1) intercambiando con la ultima particula activa
            m_pool[i] = m_pool[m_activeCount - 1];
            --m_activeCount;
        } else {
            // Actualizacion de dinamica fisica
            p.velocity += m_gravity * deltaTime;
            if (m_damping > 0.0f) {
                p.velocity *= std::max(0.0f, 1.0f - m_damping * deltaTime);
            }
            p.position += p.velocity * deltaTime;

            // Escala progresiva
            if (m_sizeGrowth != 0.0f) {
                p.size = std::max(0.01f, p.baseSize + m_sizeGrowth * (p.life - p.remainingLife));
            }

            // Desvanecimiento del canal alfa
            float lifeFraction = std::clamp(p.remainingLife / p.life, 0.0f, 1.0f);
            p.color.a = p.baseColor.a * lifeFraction;

            ++i;
        }
    }
}

bool ParticleSystem::emit(const glm::vec3& position,
                          const glm::vec3& velocity,
                          const glm::vec4& color,
                          float size,
                          float life) {
    if (m_activeCount >= m_maxParticles || life <= 0.0f) {
        return false;
    }

    Particle& p = m_pool[m_activeCount];
    p.position = position;
    p.velocity = velocity;
    p.color = color;
    p.baseColor = color;
    p.size = size;
    p.baseSize = size;
    p.life = life;
    p.remainingLife = life;

    ++m_activeCount;
    return true;
}

bool ParticleSystem::emit(const Particle& particle) {
    if (m_activeCount >= m_maxParticles || particle.life <= 0.0f) {
        return false;
    }

    Particle& p = m_pool[m_activeCount];
    p = particle;
    if (p.baseColor == glm::vec4(1.0f) && p.color != glm::vec4(1.0f)) {
        p.baseColor = p.color;
    }
    if (p.baseSize <= 0.0f) {
        p.baseSize = p.size;
    }
    if (p.remainingLife <= 0.0f) {
        p.remainingLife = p.life;
    }

    ++m_activeCount;
    return true;
}

void ParticleSystem::render(const glm::mat4& view, const glm::mat4& projection) {
    if (!m_shaderLoaded) return;
    render(m_shader, view, projection);
}

void ParticleSystem::render(const renderer::Shader& shader, const glm::mat4& view, const glm::mat4& projection) {
    if (m_activeCount == 0) return;

    if (!m_initialized) {
        initBuffers();
    }

    // Copiar atributos de particulas activas al arreglo de instancia contiguo
    for (size_t i = 0; i < m_activeCount; ++i) {
        m_instanceData[i].position = m_pool[i].position;
        m_instanceData[i].size = m_pool[i].size;
        m_instanceData[i].color = m_pool[i].color;
    }

    // Subir buffer de instancias a la GPU
    glBindBuffer(GL_ARRAY_BUFFER, m_instanceVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, m_activeCount * sizeof(ParticleInstanceData), m_instanceData.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Activacion de transparencia y desactivacion de escritura en depth mask
    glEnable(GL_BLEND);
    if (m_additive) {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    } else {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);

    shader.use();
    shader.setMat4("u_View", view);
    shader.setMat4("u_Projection", projection);
    shader.setBool("u_UseTexture", false);

    glBindVertexArray(m_quadVAO);
    glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, static_cast<GLsizei>(m_activeCount));
    glBindVertexArray(0);

    // Restaurar estado del pipeline grafico
    glEnable(GL_CULL_FACE);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

void ParticleSystem::clear() {
    m_activeCount = 0;
}

} // namespace vfx
