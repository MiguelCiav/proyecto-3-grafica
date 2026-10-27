#include "world/BoosterPad.hpp"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <algorithm>

namespace world {

// Definición de miembros estáticos para la malla unitaria compartida
unsigned int BoosterPad::s_sharedVAO = 0;
unsigned int BoosterPad::s_sharedVBO = 0;
bool BoosterPad::s_sharedMeshInitialized = false;
int BoosterPad::s_instanceCount = 0;

namespace {

const float quadVertices[] = {
    // Posición           Normal             UV
    -0.5f, 0.0f, -0.5f,   0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
    -0.5f, 0.0f,  0.5f,   0.0f, 1.0f, 0.0f,  0.0f, 1.0f,
     0.5f, 0.0f,  0.5f,   0.0f, 1.0f, 0.0f,  1.0f, 1.0f,

    -0.5f, 0.0f, -0.5f,   0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
     0.5f, 0.0f,  0.5f,   0.0f, 1.0f, 0.0f,  1.0f, 1.0f,
     0.5f, 0.0f, -0.5f,   0.0f, 1.0f, 0.0f,  1.0f, 0.0f
};

} // namespace

BoosterPad::BoosterPad()
    : m_alive(true)
{
    ++s_instanceCount;
    updateTriggerZone();
}

BoosterPad::BoosterPad(const glm::vec3& position, float yawDegrees, float width, float length)
    : m_position(position)
    , m_yawDegrees(yawDegrees)
    , m_width(width)
    , m_length(length)
    , m_alive(true)
{
    ++s_instanceCount;
    updateTriggerZone();
}

BoosterPad::~BoosterPad() {
    destroy();
}

BoosterPad::BoosterPad(BoosterPad&& other) noexcept
    : m_position(other.m_position)
    , m_yawDegrees(other.m_yawDegrees)
    , m_width(other.m_width)
    , m_length(other.m_length)
    , m_elevationOffset(other.m_elevationOffset)
    , m_boostMultiplier(other.m_boostMultiplier)
    , m_boostDuration(other.m_boostDuration)
    , m_instantImpulse(other.m_instantImpulse)
    , m_triggerZone(other.m_triggerZone)
    , m_alive(other.m_alive)
{
    other.m_alive = false;
}

BoosterPad& BoosterPad::operator=(BoosterPad&& other) noexcept {
    if (this != &other) {
        destroy();
        m_position = other.m_position;
        m_yawDegrees = other.m_yawDegrees;
        m_width = other.m_width;
        m_length = other.m_length;
        m_elevationOffset = other.m_elevationOffset;
        m_boostMultiplier = other.m_boostMultiplier;
        m_boostDuration = other.m_boostDuration;
        m_instantImpulse = other.m_instantImpulse;
        m_triggerZone = other.m_triggerZone;
        m_alive = other.m_alive;

        other.m_alive = false;
    }
    return *this;
}

void BoosterPad::setPosition(const glm::vec3& pos) {
    m_position = pos;
    updateTriggerZone();
}

void BoosterPad::setYaw(float yawDegrees) {
    m_yawDegrees = yawDegrees;
    updateTriggerZone();
}

void BoosterPad::setDimensions(float width, float length) {
    m_width = width;
    m_length = length;
    updateTriggerZone();
}

void BoosterPad::setBoostMultiplier(float multiplier) {
    m_boostMultiplier = multiplier;
    updateTriggerZone();
}

void BoosterPad::setBoostDuration(float duration) {
    m_boostDuration = duration;
    updateTriggerZone();
}

bool BoosterPad::contains(const glm::vec3& point) const {
    return m_triggerZone.contains(point);
}

bool BoosterPad::onKartContact(Engine::Physics::KartPhysics& kart) const {
    if (contains(kart.getPosition())) {
        kart.applyBoost(m_boostDuration, m_boostMultiplier, m_instantImpulse);
        return true;
    }
    return false;
}

void BoosterPad::registerTrigger(ZoneTriggerManager& manager) const {
    manager.addZone(m_triggerZone);
}

void BoosterPad::initSharedMesh() {
    if (s_sharedMeshInitialized) return;
    if (!glad_glGenVertexArrays) return;

    glGenVertexArrays(1, &s_sharedVAO);
    glGenBuffers(1, &s_sharedVBO);

    glBindVertexArray(s_sharedVAO);
    glBindBuffer(GL_ARRAY_BUFFER, s_sharedVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    // Posición (Location 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), reinterpret_cast<void*>(0));

    // Normal (Location 1)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));

    // UVs (Location 2)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), reinterpret_cast<void*>(6 * sizeof(float)));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    s_sharedMeshInitialized = true;
}

void BoosterPad::destroySharedMesh() {
    if (!s_sharedMeshInitialized && s_sharedVAO == 0 && s_sharedVBO == 0) {
        return;
    }

    if (glfwGetCurrentContext() != nullptr) {
        if (s_sharedVAO != 0) {
            glDeleteVertexArrays(1, &s_sharedVAO);
        }
        if (s_sharedVBO != 0) {
            glDeleteBuffers(1, &s_sharedVBO);
        }
    }

    s_sharedVAO = 0;
    s_sharedVBO = 0;
    s_sharedMeshInitialized = false;
}

void BoosterPad::updateTriggerZone() {
    float halfW = m_width * 0.5f;
    float halfL = m_length * 0.5f;

    float rad = glm::radians(m_yawDegrees);
    float cosA = std::cos(rad);
    float sinA = std::sin(rad);

    glm::vec2 corners[4] = {
        {-halfW, -halfL},
        { halfW, -halfL},
        { halfW,  halfL},
        {-halfW,  halfL}
    };

    float minX = 1e9f, maxX = -1e9f;
    float minZ = 1e9f, maxZ = -1e9f;

    for (int i = 0; i < 4; ++i) {
        float rotX = corners[i].x * cosA + corners[i].y * sinA;
        float rotZ = -corners[i].x * sinA + corners[i].y * cosA;

        float worldX = m_position.x + rotX;
        float worldZ = m_position.z + rotZ;

        minX = std::min(minX, worldX);
        maxX = std::max(maxX, worldX);
        minZ = std::min(minZ, worldZ);
        maxZ = std::max(maxZ, worldZ);
    }

    m_triggerZone.type = ZoneType::BOOST;
    m_triggerZone.minBounds = glm::vec3(minX, m_position.y - 0.2f, minZ);
    m_triggerZone.maxBounds = glm::vec3(maxX, m_position.y + 1.2f, maxZ);
    m_triggerZone.speedMultiplier = m_boostMultiplier;
    m_triggerZone.durationSeconds = m_boostDuration;
}

void BoosterPad::draw(const renderer::Shader& proceduralShader, float currentTime) const {
    if (!m_alive) return;

    if (!s_sharedMeshInitialized) {
        initSharedMesh();
    }
    if (!s_sharedMeshInitialized || s_sharedVAO == 0 || !proceduralShader.isValid()) return;

    glm::mat4 model = glm::translate(glm::mat4(1.0f), m_position + glm::vec3(0.0f, m_elevationOffset, 0.0f));
    model = glm::rotate(model, glm::radians(m_yawDegrees), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(m_width, 1.0f, m_length));

    proceduralShader.use();
    proceduralShader.setMat4("u_Model", model);
    proceduralShader.setFloat("u_Time", currentTime);

    glBindVertexArray(s_sharedVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void BoosterPad::destroy() {
    if (m_alive) {
        m_alive = false;
        if (s_instanceCount > 0) {
            --s_instanceCount;
            if (s_instanceCount == 0) {
                destroySharedMesh();
            }
        }
    }
}

} // namespace world
