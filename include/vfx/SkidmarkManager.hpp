#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <deque>
#include <string>
#include "renderer/Shader.hpp"

namespace vfx {

struct SkidVertex {
    glm::vec3 position;
    glm::vec4 color;
    glm::vec2 uv;
};

struct SkidSegment {
    SkidVertex v[4];
    float lifetime{12.0f}; // Segundos antes de desvanecerse por completo
    float age{0.0f};
    float maxAlpha{0.75f};
};

struct WheelTrailState {
    bool hasLastEdge{false};
    glm::vec3 lastLeftEdge{0.0f};
    glm::vec3 lastRightEdge{0.0f};
    glm::vec3 lastPos{0.0f};
};

/**
 * @brief Gestor procedural de marcas de neumaticos y derrape en el asfalto.
 * Genera cintas de cuadrilateros continuas y suavizadas directamente en el punto
 * exacto de contacto de las ruedas traseras con la pista.
 */
class SkidmarkManager {
public:
    explicit SkidmarkManager(size_t maxSegments = 1600);
    ~SkidmarkManager();

    SkidmarkManager(const SkidmarkManager&) = delete;
    SkidmarkManager& operator=(const SkidmarkManager&) = delete;

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
    [[nodiscard]] size_t getActiveSegmentCount() const { return m_segments.size(); }

private:
    void addWheelPoint(int wheelIdx, const glm::vec3& currentPos, float intensity);

    size_t m_maxSegments{1600};
    std::deque<SkidSegment> m_segments;

    bool m_currentlySkidding{false};
    WheelTrailState m_wheelTrails[2];

    // Geometria GPU dinamica
    unsigned int m_vao{0};
    unsigned int m_vbo{0};
    unsigned int m_ebo{0};
    renderer::Shader m_shader;
    bool m_initialized{false};

    std::vector<SkidVertex> m_vertexCache;
    std::vector<unsigned int> m_indexCache;
};

} // namespace vfx
