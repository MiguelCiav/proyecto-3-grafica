#include "vfx/SkidmarkManager.hpp"
#include <glad/glad.h>
#include <cmath>
#include <algorithm>
#include <iostream>

namespace vfx {

namespace {

const char* SKID_VERT_SRC = R"(#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec4 aColor;

uniform mat4 u_View;
uniform mat4 u_Projection;

out vec4 vColor;

void main() {
    vColor = aColor;
    gl_Position = u_Projection * u_View * vec4(aPos, 1.0);
}
)";

const char* SKID_FRAG_SRC = R"(#version 330 core
in vec4 vColor;
out vec4 FragColor;

void main() {
    FragColor = vColor;
}
)";

} // namespace

SkidmarkManager::SkidmarkManager(size_t maxSegments)
    : m_maxSegments(maxSegments) {
    m_vertexCache.reserve(maxSegments * 4);
    m_indexCache.reserve(maxSegments * 6);
}

SkidmarkManager::~SkidmarkManager() {
    if (m_vao) glDeleteVertexArrays(1, &m_vao);
    if (m_vbo) glDeleteBuffers(1, &m_vbo);
    if (m_ebo) glDeleteBuffers(1, &m_ebo);
}

bool SkidmarkManager::init() {
    if (m_initialized) return true;

    if (!m_shader.loadFromSource(SKID_VERT_SRC, SKID_FRAG_SRC)) {
        std::cerr << "[ERROR] [SkidmarkManager] Error compilando shader procedural de marcas de derrape." << std::endl;
        return false;
    }

    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    glGenBuffers(1, &m_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, m_maxSegments * 4 * sizeof(SkidVertex), nullptr, GL_DYNAMIC_DRAW);

    glGenBuffers(1, &m_ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_maxSegments * 6 * sizeof(unsigned int), nullptr, GL_DYNAMIC_DRAW);

    // Atributo 0: Posicion 3D (x, y, z)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SkidVertex), reinterpret_cast<void*>(offsetof(SkidVertex, position)));

    // Atributo 1: Color y Alpha (r, g, b, a)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(SkidVertex), reinterpret_cast<void*>(offsetof(SkidVertex, color)));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    m_initialized = true;
    std::cout << "[OK] [SkidmarkManager] Sistema de marcas de derrape inicializado (capacidad: "
              << m_maxSegments << " segmentos)." << std::endl;
    return true;
}

void SkidmarkManager::clear() {
    m_segments.clear();
    m_hasLastPos[0] = false;
    m_hasLastPos[1] = false;
    m_currentlySkidding = false;
}

void SkidmarkManager::addWheelSegment(int wheelIdx, const glm::vec3& currentPos, float intensity) {
    if (!m_hasLastPos[wheelIdx]) {
        m_lastPos[wheelIdx] = currentPos;
        m_hasLastPos[wheelIdx] = true;
        return;
    }

    float dist = glm::distance(currentPos, m_lastPos[wheelIdx]);
    if (dist < 0.18f) {
        // Demasiado cerca, esperar siguiente fotograma para evitar sobrecarga de geometria
        return;
    }
    if (dist > 3.0f) {
        // Teletransporte o salto brusco: reiniciar rastro
        m_lastPos[wheelIdx] = currentPos;
        return;
    }

    glm::vec3 delta = currentPos - m_lastPos[wheelIdx];
    glm::vec3 dir = glm::normalize(delta);
    const float tireWidth = 0.23f;
    glm::vec3 perp = glm::cross(dir, glm::vec3(0.0f, 1.0f, 0.0f)) * (tireWidth * 0.5f);

    float alpha = std::clamp(intensity * 0.75f, 0.25f, 0.85f);
    glm::vec4 rubberColor(0.06f, 0.06f, 0.07f, alpha);

    SkidSegment seg;
    seg.lifetime = 12.0f; // 12 segundos de desvanecimiento suave
    seg.age = 0.0f;
    seg.maxAlpha = alpha;

    seg.v[0] = {m_lastPos[wheelIdx] - perp, rubberColor};
    seg.v[1] = {m_lastPos[wheelIdx] + perp, rubberColor};
    seg.v[2] = {currentPos + perp, rubberColor};
    seg.v[3] = {currentPos - perp, rubberColor};

    if (m_segments.size() >= m_maxSegments) {
        m_segments.pop_front();
    }
    m_segments.push_back(seg);

    m_lastPos[wheelIdx] = currentPos;
}

void SkidmarkManager::update(float deltaTime, const glm::vec3& kartPos, float kartYawDeg,
                             float speed, float steerInput, bool isBraking, bool isBoosting) {
    // 1. Envejecimiento y desvanecimiento de marcas existentes
    while (!m_segments.empty() && m_segments.front().age >= m_segments.front().lifetime) {
        m_segments.pop_front();
    }

    for (auto& seg : m_segments) {
        seg.age += deltaTime;
        float remaining = 1.0f - (seg.age / seg.lifetime);
        float currentAlpha = seg.maxAlpha * std::clamp(remaining * remaining, 0.0f, 1.0f);
        seg.v[0].color.a = currentAlpha;
        seg.v[1].color.a = currentAlpha;
        seg.v[2].color.a = currentAlpha;
        seg.v[3].color.a = currentAlpha;
    }

    // 2. Evaluacion de derrape y friccion limite
    bool hardTurn = (std::abs(steerInput) > 0.25f && speed > 5.0f);
    bool brakeSkid = (isBraking && speed > 3.0f);
    bool boostDrift = (isBoosting && std::abs(steerInput) > 0.12f && speed > 4.0f);

    m_currentlySkidding = (hardTurn || brakeSkid || boostDrift);

    if (m_currentlySkidding) {
        float rad = glm::radians(kartYawDeg);
        glm::vec3 fwd(std::sin(rad), 0.0f, -std::cos(rad));
        glm::vec3 right(std::cos(rad), 0.0f, std::sin(rad));

        float backOffset = 0.65f;
        float halfTrack = 0.45f;
        float groundY = 0.015f; // Ligeramente elevado sobre y=0 para evitar Z-fighting

        glm::vec3 leftTirePos = kartPos - fwd * backOffset - right * halfTrack;
        glm::vec3 rightTirePos = kartPos - fwd * backOffset + right * halfTrack;
        leftTirePos.y = groundY;
        rightTirePos.y = groundY;

        float intensity = 0.6f;
        if (hardTurn) intensity += std::min(std::abs(steerInput) * 0.4f, 0.4f);
        if (brakeSkid) intensity += 0.35f;
        if (boostDrift) intensity += 0.40f;

        addWheelSegment(0, leftTirePos, intensity);
        addWheelSegment(1, rightTirePos, intensity);
    } else {
        m_hasLastPos[0] = false;
        m_hasLastPos[1] = false;
    }
}

void SkidmarkManager::render(const glm::mat4& view, const glm::mat4& projection) {
    if (!m_initialized || m_segments.empty()) return;

    m_vertexCache.clear();
    m_indexCache.clear();

    unsigned int baseIdx = 0;
    for (const auto& seg : m_segments) {
        m_vertexCache.push_back(seg.v[0]);
        m_vertexCache.push_back(seg.v[1]);
        m_vertexCache.push_back(seg.v[2]);
        m_vertexCache.push_back(seg.v[3]);

        m_indexCache.push_back(baseIdx + 0);
        m_indexCache.push_back(baseIdx + 1);
        m_indexCache.push_back(baseIdx + 2);
        m_indexCache.push_back(baseIdx + 0);
        m_indexCache.push_back(baseIdx + 2);
        m_indexCache.push_back(baseIdx + 3);

        baseIdx += 4;
    }

    // Subir geometria dinamica
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, m_vertexCache.size() * sizeof(SkidVertex), m_vertexCache.data());

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, m_indexCache.size() * sizeof(unsigned int), m_indexCache.data());

    // Configurar estado OpenGL para calcomanias sobre el asfalto
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE); // Translucidez sin ocluir objetos posteriores
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(-2.0f, -2.0f);

    m_shader.use();
    m_shader.setMat4("u_View", view);
    m_shader.setMat4("u_Projection", projection);

    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indexCache.size()), GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);

    glDisable(GL_POLYGON_OFFSET_FILL);
    glDepthMask(GL_TRUE);
}

} // namespace vfx
