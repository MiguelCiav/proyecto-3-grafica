#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <string>
#include "renderer/Shader.hpp"

namespace vfx {

struct SkidVertex {
    glm::vec3 position{0.0f};
    glm::vec4 color{1.0f};
    glm::vec2 uv{0.0f};
    float spawnTime{0.0f};
};

struct SkidSegment {
    SkidVertex v[4];
    float lifetime{4.0f}; // Segundos antes de desvanecerse por completo
    float age{0.0f};
    float maxAlpha{0.75f};
    float spawnTime{0.0f};
};

struct WheelTrailState {
    bool hasLastEdge{false};
    glm::vec3 lastLeftEdge{0.0f};
    glm::vec3 lastRightEdge{0.0f};
    glm::vec3 lastPos{0.0f};
};

/**
 * @brief Gestor procedural de marcas de neumaticos y derrape en el asfalto.
 * Implementa un Ring Buffer circular continuo en GPU con desvanecimiento
 * analítico en shader (GPU-based fade), eliminando re-uploads masivos de vértices por PCIe.
 */
class SkidmarkManager {
public:
    explicit SkidmarkManager(size_t maxSegments = 1600);
    ~SkidmarkManager();

    // No copiable
    SkidmarkManager(const SkidmarkManager&) = delete;
    SkidmarkManager& operator=(const SkidmarkManager&) = delete;

    // Movible con garantías noexcept (Regla de los 5 y RAII)
    SkidmarkManager(SkidmarkManager&& other) noexcept;
    SkidmarkManager& operator=(SkidmarkManager&& other) noexcept;

    bool init(const std::string& vertPath = "shaders/skidmark.vert",
              const std::string& fragPath = "shaders/skidmark.frag");

    // Actualizacion pasando posicion y orientacion global del vehiculo
    void update(float deltaTime, const glm::vec3& kartPos, float kartYawDeg,
                float speed, float steerInput, bool isBraking, bool isBoosting);

    // Actualizacion de maxima precision pasando las coordenadas de contacto exactas de las ruedas
    void updateWithWheelPositions(float deltaTime,
                                  const glm::vec3& leftTirePos,
                                  const glm::vec3& rightTirePos,
                                  float speed, float steerInput, bool isBraking, bool isBoosting);

    void render(const glm::mat4& view, const glm::mat4& projection);
    void clear();

    [[nodiscard]] bool isCurrentlySkidding() const { return m_currentlySkidding; }
    [[nodiscard]] size_t getActiveSegmentCount() const { return m_count; }
    [[nodiscard]] size_t getMaxSegments() const { return m_maxSegments; }

    void setFadeDuration(float duration) { m_fadeDuration = duration; }
    [[nodiscard]] float getFadeDuration() const { return m_fadeDuration; }

private:
    void addWheelPoint(int wheelIdx, const glm::vec3& currentPos, float intensity);

    size_t m_maxSegments{1600};
    size_t m_headIndex{0};
    size_t m_count{0};
    float m_totalTime{0.0f};
    float m_fadeDuration{4.0f};

    std::vector<SkidSegment> m_segments;

    bool m_currentlySkidding{false};
    WheelTrailState m_wheelTrails[2];

    // Geometria GPU dinamica
    unsigned int m_vao{0};
    unsigned int m_vbo{0};
    unsigned int m_ebo{0};
    renderer::Shader m_shader;
    bool m_initialized{false};
};

} // namespace vfx
