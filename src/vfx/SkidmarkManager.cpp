#include "vfx/SkidmarkManager.hpp"
#include <glad/glad.h>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <cstdint>

namespace vfx {

SkidmarkManager::SkidmarkManager(size_t maxSegments)
    : m_maxSegments(maxSegments == 0 ? 1600 : maxSegments),
      m_headIndex(0),
      m_count(0),
      m_totalTime(0.0f),
      m_fadeDuration(4.0f),
      m_segments(maxSegments == 0 ? 1600 : maxSegments) {
}

SkidmarkManager::~SkidmarkManager() {
    if (m_vao) glDeleteVertexArrays(1, &m_vao);
    if (m_vbo) glDeleteBuffers(1, &m_vbo);
    if (m_ebo) glDeleteBuffers(1, &m_ebo);
}

SkidmarkManager::SkidmarkManager(SkidmarkManager&& other) noexcept
    : m_maxSegments(other.m_maxSegments),
      m_headIndex(other.m_headIndex),
      m_count(other.m_count),
      m_totalTime(other.m_totalTime),
      m_fadeDuration(other.m_fadeDuration),
      m_segments(std::move(other.m_segments)),
      m_currentlySkidding(other.m_currentlySkidding),
      m_vao(other.m_vao),
      m_vbo(other.m_vbo),
      m_ebo(other.m_ebo),
      m_shader(std::move(other.m_shader)),
      m_initialized(other.m_initialized) {
    m_wheelTrails[0] = other.m_wheelTrails[0];
    m_wheelTrails[1] = other.m_wheelTrails[1];

    other.m_vao = 0;
    other.m_vbo = 0;
    other.m_ebo = 0;
    other.m_initialized = false;
    other.m_count = 0;
    other.m_headIndex = 0;
}

SkidmarkManager& SkidmarkManager::operator=(SkidmarkManager&& other) noexcept {
    if (this != &other) {
        if (m_vao) glDeleteVertexArrays(1, &m_vao);
        if (m_vbo) glDeleteBuffers(1, &m_vbo);
        if (m_ebo) glDeleteBuffers(1, &m_ebo);

        m_maxSegments = other.m_maxSegments;
        m_headIndex = other.m_headIndex;
        m_count = other.m_count;
        m_totalTime = other.m_totalTime;
        m_fadeDuration = other.m_fadeDuration;
        m_segments = std::move(other.m_segments);
        m_currentlySkidding = other.m_currentlySkidding;
        m_wheelTrails[0] = other.m_wheelTrails[0];
        m_wheelTrails[1] = other.m_wheelTrails[1];
        m_vao = other.m_vao;
        m_vbo = other.m_vbo;
        m_ebo = other.m_ebo;
        m_shader = std::move(other.m_shader);
        m_initialized = other.m_initialized;

        other.m_vao = 0;
        other.m_vbo = 0;
        other.m_ebo = 0;
        other.m_initialized = false;
        other.m_count = 0;
        other.m_headIndex = 0;
    }
    return *this;
}

bool SkidmarkManager::init(const std::string& vertPath, const std::string& fragPath) {
    if (m_initialized) return true;

    if (!m_shader.loadFromFiles(vertPath, fragPath)) {
        std::cerr << "[ERROR] [SkidmarkManager] Error cargando shaders desde: "
                  << vertPath << " y " << fragPath << std::endl;
        return false;
    }

    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    glGenBuffers(1, &m_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(m_maxSegments * 4 * sizeof(SkidVertex)),
                 nullptr,
                 GL_DYNAMIC_DRAW);

    // Pre-poblar índices EBO estáticos una sola vez (0,1,2, 0,2,3 para cada quad)
    std::vector<unsigned int> staticIndices;
    staticIndices.reserve(m_maxSegments * 6);
    for (size_t i = 0; i < m_maxSegments; ++i) {
        unsigned int base = static_cast<unsigned int>(i * 4);
        staticIndices.push_back(base + 0);
        staticIndices.push_back(base + 1);
        staticIndices.push_back(base + 2);
        staticIndices.push_back(base + 0);
        staticIndices.push_back(base + 2);
        staticIndices.push_back(base + 3);
    }

    glGenBuffers(1, &m_ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(staticIndices.size() * sizeof(unsigned int)),
                 staticIndices.data(),
                 GL_STATIC_DRAW);

    // Atributo 0: Posicion 3D (x, y, z)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SkidVertex),
                          reinterpret_cast<void*>(offsetof(SkidVertex, position)));

    // Atributo 1: Color y Alpha base (r, g, b, a)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(SkidVertex),
                          reinterpret_cast<void*>(offsetof(SkidVertex, color)));

    // Atributo 2: Coordenadas UV (u, v) para suavizado de bordes y estriado de neumático
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(SkidVertex),
                          reinterpret_cast<void*>(offsetof(SkidVertex, uv)));

    // Atributo 3: Spawn time (float) para cálculo analítico de desvanecimiento en GPU
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(SkidVertex),
                          reinterpret_cast<void*>(offsetof(SkidVertex, spawnTime)));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    m_initialized = true;
    std::cout << "[OK] [SkidmarkManager] Sistema de marcas de derrape inicializado (Ring Buffer: "
              << m_maxSegments << " segmentos continuos, GPU decay)." << std::endl;
    return true;
}

void SkidmarkManager::clear() {
    m_count = 0;
    m_headIndex = 0;
    m_wheelTrails[0] = WheelTrailState{};
    m_wheelTrails[1] = WheelTrailState{};
    m_currentlySkidding = false;
}

void SkidmarkManager::addWheelPoint(int wheelIdx, const glm::vec3& currentPos, float intensity) {
    auto& trail = m_wheelTrails[wheelIdx];

    // Primer punto del rastro: guardar posición y esperar siguiente fotograma
    if (!trail.hasLastEdge && trail.lastPos == glm::vec3(0.0f)) {
        trail.lastPos = currentPos;
        return;
    }

    float dist = glm::distance(currentPos, trail.lastPos);
    if (dist < 0.12f) {
        // La distancia recorrida aún es muy pequeña para formar un nuevo cuadrilátero
        return;
    }
    if (dist > 2.5f) {
        // Salto brusco, reset o respawn del vehículo: romper la cinta limpiamente
        trail.hasLastEdge = false;
        trail.lastPos = currentPos;
        return;
    }

    glm::vec3 delta = currentPos - trail.lastPos;
    glm::vec3 travelDir = glm::normalize(delta);

    // Vector perpendicular en el plano horizontal (XZ)
    const float tireWidth = 0.19f;
    glm::vec3 perp = glm::cross(travelDir, glm::vec3(0.0f, 1.0f, 0.0f));
    if (glm::dot(perp, perp) > 0.0001f) {
        perp = glm::normalize(perp) * (tireWidth * 0.5f);
    } else {
        perp = glm::vec3(tireWidth * 0.5f, 0.0f, 0.0f);
    }

    // Extremos del segmento actual
    glm::vec3 curLeft = currentPos - perp;
    glm::vec3 curRight = currentPos + perp;

    // Extremos del segmento previo: si ya venía derrapando, se enlaza directamente
    // con los vértices finales del segmento anterior para formar una cinta 100% continua
    glm::vec3 prevLeft = trail.hasLastEdge ? trail.lastLeftEdge : (trail.lastPos - perp);
    glm::vec3 prevRight = trail.hasLastEdge ? trail.lastRightEdge : (trail.lastPos + perp);

    float alpha = std::clamp(intensity * 0.72f, 0.28f, 0.85f);
    glm::vec4 rubberColor(0.04f, 0.04f, 0.05f, alpha); // Negro carbón de caucho caliente

    SkidSegment seg;
    seg.lifetime = m_fadeDuration;
    seg.age = 0.0f;
    seg.maxAlpha = alpha;
    seg.spawnTime = m_totalTime;

    // Cuadrilátero continuo con mapeo UV y tiempo de creación para GPU decay
    seg.v[0] = {prevLeft, rubberColor, glm::vec2(0.0f, 0.0f), m_totalTime};
    seg.v[1] = {prevRight, rubberColor, glm::vec2(1.0f, 0.0f), m_totalTime};
    seg.v[2] = {curRight, rubberColor, glm::vec2(1.0f, 1.0f), m_totalTime};
    seg.v[3] = {curLeft, rubberColor, glm::vec2(0.0f, 1.0f), m_totalTime};

    size_t writeIdx = m_headIndex;
    m_segments[writeIdx] = seg;
    m_headIndex = (m_headIndex + 1) % m_maxSegments;
    if (m_count < m_maxSegments) {
        m_count++;
    }

    // Subir a la GPU únicamente el nuevo segmento (< 1 KB) en lugar de todo el buffer
    if (m_initialized && m_vbo) {
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferSubData(GL_ARRAY_BUFFER,
                        static_cast<GLintptr>(writeIdx * 4 * sizeof(SkidVertex)),
                        static_cast<GLsizeiptr>(4 * sizeof(SkidVertex)),
                        seg.v);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    // Actualizar extremos para la siguiente iteración
    trail.lastLeftEdge = curLeft;
    trail.lastRightEdge = curRight;
    trail.lastPos = currentPos;
    trail.hasLastEdge = true;
}

void SkidmarkManager::updateWithWheelPositions(float deltaTime,
                                              const glm::vec3& leftTirePos,
                                              const glm::vec3& rightTirePos,
                                              float speed, float steerInput, bool isBraking, bool isBoosting) {
    m_totalTime += deltaTime;

    // 1. Envejecimiento y desvanecimiento progresivo: retirar segmentos expirados en O(1)
    while (m_count > 0) {
        size_t tailIdx = (m_headIndex + m_maxSegments - m_count) % m_maxSegments;
        if ((m_totalTime - m_segments[tailIdx].spawnTime) >= m_fadeDuration) {
            m_count--;
        } else {
            break;
        }
    }

    // 2. Detección física natural de derrape y pérdida de tracción
    float absSteer = std::abs(steerInput);
    bool hardTurn = (absSteer > 0.28f && speed > 5.2f);
    bool brakeSkid = (isBraking && speed > 1.8f);
    bool boostDrift = (isBoosting && absSteer > 0.10f && speed > 3.8f);

    m_currentlySkidding = (hardTurn || brakeSkid || boostDrift);

    if (m_currentlySkidding) {
        float intensity = 0.55f;
        if (hardTurn) intensity += std::min(absSteer * 0.45f, 0.45f);
        if (brakeSkid) intensity += 0.40f;
        if (boostDrift) intensity += 0.45f;

        addWheelPoint(0, leftTirePos, intensity);
        addWheelPoint(1, rightTirePos, intensity);
    } else {
        // Cortar la continuidad de la cinta al dejar de derrapar
        m_wheelTrails[0].hasLastEdge = false;
        m_wheelTrails[0].lastPos = glm::vec3(0.0f);
        m_wheelTrails[1].hasLastEdge = false;
        m_wheelTrails[1].lastPos = glm::vec3(0.0f);
    }
}

void SkidmarkManager::update(float deltaTime, const glm::vec3& kartPos, float kartYawDeg,
                             float speed, float steerInput, bool isBraking, bool isBoosting) {
    // Conversión matemáticamente rigurosa desde la orientación y posición del vehículo
    // hacia los puntos de contacto al suelo de las ruedas traseras (Z = -0.361m, X = +/-0.382m)
    float rad = glm::radians(kartYawDeg);
    float cosY = std::cos(rad);
    float sinY = std::sin(rad);

    auto localToWorld = [&](float lx, float lz) {
        float wx = lx * cosY + lz * sinY;
        float wz = -lx * sinY + lz * cosY;
        return kartPos + glm::vec3(wx, 0.012f, wz);
    };

    glm::vec3 leftTirePos = localToWorld(0.3823f, -0.3606f);
    glm::vec3 rightTirePos = localToWorld(-0.3823f, -0.3606f);

    updateWithWheelPositions(deltaTime, leftTirePos, rightTirePos, speed, steerInput, isBraking, isBoosting);
}

void SkidmarkManager::render(const glm::mat4& view, const glm::mat4& projection) {
    if (!m_initialized || m_count == 0) return;

    // Configuración de renderizado para calcomanías sobre el asfalto
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(-2.0f, -2.0f);

    m_shader.use();
    m_shader.setMat4("u_View", view);
    m_shader.setMat4("u_Projection", projection);
    m_shader.setFloat("u_CurrentTime", m_totalTime);
    m_shader.setFloat("u_FadeDuration", m_fadeDuration);

    glBindVertexArray(m_vao);

    size_t tail = (m_headIndex + m_maxSegments - m_count) % m_maxSegments;
    if (tail + m_count <= m_maxSegments) {
        // Bloque contiguo único
        uintptr_t byteOffset = tail * 6 * sizeof(unsigned int);
        glDrawElements(GL_TRIANGLES,
                       static_cast<GLsizei>(m_count * 6),
                       GL_UNSIGNED_INT,
                       reinterpret_cast<void*>(byteOffset));
    } else {
        // Dos bloques contiguos debido al wrap-around del ring buffer
        size_t firstCount = m_maxSegments - tail;
        size_t secondCount = m_count - firstCount;

        uintptr_t firstOffset = tail * 6 * sizeof(unsigned int);
        glDrawElements(GL_TRIANGLES,
                       static_cast<GLsizei>(firstCount * 6),
                       GL_UNSIGNED_INT,
                       reinterpret_cast<void*>(firstOffset));

        glDrawElements(GL_TRIANGLES,
                       static_cast<GLsizei>(secondCount * 6),
                       GL_UNSIGNED_INT,
                       nullptr);
    }

    glBindVertexArray(0);

    glDisable(GL_POLYGON_OFFSET_FILL);
    glEnable(GL_CULL_FACE);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

} // namespace vfx
