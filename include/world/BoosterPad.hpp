#pragma once

#include "world/ZoneTrigger.hpp"
#include "renderer/Shader.hpp"
#include "physics/KartPhysics.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace world {

class BoosterPad {
public:
    BoosterPad();
    BoosterPad(const glm::vec3& position, float yawDegrees = 0.0f, float width = 3.0f, float length = 4.5f);
    ~BoosterPad();

    BoosterPad(const BoosterPad&) = delete;
    BoosterPad& operator=(const BoosterPad&) = delete;

    BoosterPad(BoosterPad&& other) noexcept;
    BoosterPad& operator=(BoosterPad&& other) noexcept;

    void setPosition(const glm::vec3& pos);
    const glm::vec3& getPosition() const { return m_position; }

    void setYaw(float yawDegrees);
    float getYaw() const { return m_yawDegrees; }

    void setDimensions(float width, float length);
    float getWidth() const { return m_width; }
    float getLength() const { return m_length; }

    void setBoostMultiplier(float multiplier);
    float getBoostMultiplier() const { return m_boostMultiplier; }

    void setBoostDuration(float duration);
    float getBoostDuration() const { return m_boostDuration; }

    void setInstantImpulse(float impulse) { m_instantImpulse = impulse; }
    float getInstantImpulse() const { return m_instantImpulse; }

    const SpecialZone& getTriggerZone() const { return m_triggerZone; }
    bool contains(const glm::vec3& point) const;

    bool onKartContact(Engine::Physics::KartPhysics& kart) const;
    void registerTrigger(ZoneTriggerManager& manager) const;

    void draw(const renderer::Shader& proceduralShader, float currentTime) const;

    void destroy();

    // Gestión de malla compartida (Zero Blender - Unit Quad en GPU)
    static void initSharedMesh();
    static void destroySharedMesh();
    static bool isSharedMeshInitialized() { return s_sharedMeshInitialized; }
    static unsigned int getSharedVAO() { return s_sharedVAO; }
    static unsigned int getSharedVBO() { return s_sharedVBO; }
    static int getInstanceCount() { return s_instanceCount; }

private:
    void updateTriggerZone();

    glm::vec3 m_position{0.0f};
    float m_yawDegrees{0.0f};
    float m_width{3.0f};
    float m_length{4.5f};
    float m_elevationOffset{0.085f};

    float m_boostMultiplier{1.28f};
    float m_boostDuration{1.2f};
    float m_instantImpulse{3.5f};

    SpecialZone m_triggerZone;

    bool m_alive{true};

    // Recursos de malla compartida estática
    static unsigned int s_sharedVAO;
    static unsigned int s_sharedVBO;
    static bool s_sharedMeshInitialized;
    static int s_instanceCount;
};

} // namespace world
