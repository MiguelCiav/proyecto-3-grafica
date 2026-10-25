#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <deque>
#include "renderer/Shader.hpp"

namespace vfx {

struct SkidVertex {
    glm::vec3 position;
    glm::vec4 color;
};

struct SkidSegment {
    SkidVertex v[4];
    float lifetime{14.0f}; // Segundos antes de desvanecerse por completo
    float age{0.0f};
    float maxAlpha{0.75f};
};

/**
 * @brief Gestor procedural de marcas de neumaticos y derrape en el asfalto.
 * Genera cintas de cuadrilateros translucidas detrás de las ruedas traseras
 * cuando el vehiculo derrapa, gira bruscamente o frena a alta velocidad.
 */
class SkidmarkManager {
public:
    explicit SkidmarkManager(size_t maxSegments = 1200);
    ~SkidmarkManager();

    SkidmarkManager(const SkidmarkManager&) = delete;
    SkidmarkManager& operator=(const SkidmarkManager&) = delete;

    bool init();
    void update(float deltaTime, const glm::vec3& kartPos, float kartYawDeg,
                float speed, float steerInput, bool isBraking, bool isBoosting);
    void render(const glm::mat4& view, const glm::mat4& projection);
    void clear();

    [[nodiscard]] bool isCurrentlySkidding() const { return m_currentlySkidding; }
    [[nodiscard]] size_t getActiveSegmentCount() const { return m_segments.size(); }

private:
    void addWheelSegment(int wheelIdx, const glm::vec3& currentPos, float intensity);

    size_t m_maxSegments{1200};
    std::deque<SkidSegment> m_segments;

    bool m_currentlySkidding{false};
    bool m_hasLastPos[2]{false, false};
    glm::vec3 m_lastPos[2]{glm::vec3(0.0f), glm::vec3(0.0f)};

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
