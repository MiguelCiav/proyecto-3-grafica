#pragma once

#include "lighting/Light.hpp"
#include <glm/glm.hpp>
#include <vector>
#include <string>

namespace renderer {
    class Shader;
}

namespace lighting {

/**
 * @brief Gestor central de iluminación para la escena (Fase 7 - Issue #25).
 * Administra:
 * - 1 Luz Direccional (sol de la pista).
 * - Múltiples Luces Puntuales dinámicas (farolas del circuito con atenuación calculada).
 * - 1 Luz Focal / Spotlight (faros delanteros del kart).
 * - Propiedades de material y conmutador de modelo de sombreado (Flat, Lambert, Blinn-Phong).
 */
class LightManager {
public:
    static constexpr size_t MAX_POINT_LIGHTS = 8;

    LightManager();
    ~LightManager() = default;

    // Configuración inicial de las fuentes de iluminación con valores balanceados
    void init();

    // -------------------------------------------------------------
    // Gestión de Luz Direccional (Sol)
    // -------------------------------------------------------------
    void setDirectionalLight(const DirectionalLight& light) { m_dirLight = light; }
    DirectionalLight& getDirectionalLight() { return m_dirLight; }
    const DirectionalLight& getDirectionalLight() const { return m_dirLight; }
    void toggleDirectionalLight() { m_dirLight.enabled = !m_dirLight.enabled; }

    // -------------------------------------------------------------
    // Gestión de Múltiples Luces Puntuales (Farolas de pista)
    // -------------------------------------------------------------
    void addPointLight(const PointLight& light);
    void clearPointLights();
    size_t getPointLightCount() const { return m_pointLights.size(); }
    PointLight& getPointLight(size_t index);
    const PointLight& getPointLight(size_t index) const;
    const std::vector<PointLight>& getPointLights() const { return m_pointLights; }
    void togglePointLights();

    // -------------------------------------------------------------
    // Gestión de Luz Focal / Spotlight (Faros del kart)
    // -------------------------------------------------------------
    void setSpotLight(const SpotLight& light) { m_spotLight = light; }
    SpotLight& getSpotLight() { return m_spotLight; }
    const SpotLight& getSpotLight() const { return m_spotLight; }
    void toggleSpotLight() { m_spotLight.enabled = !m_spotLight.enabled; }

    // Actualiza la posición y orientación de los faros frontales anclados al morro del kart
    void updateKartHeadlights(const glm::vec3& kartPos, const glm::vec3& forwardDir);

    // -------------------------------------------------------------
    // Conmutador del Modelo de Iluminación (Requisito Cátedra UCV)
    // -------------------------------------------------------------
    void setLightingModel(LightingModel model);
    void cycleNextModel();
    LightingModel getLightingModel() const { return m_currentModel; }
    const char* getCurrentModelName() const { return getLightingModelName(m_currentModel); }

    // -------------------------------------------------------------
    // Material
    // -------------------------------------------------------------
    const Material& getDefaultMaterial() const { return m_defaultMaterial; }
    void setMaterialShininess(float shininess) { m_defaultMaterial.shininess = shininess; }

    // -------------------------------------------------------------
    // Serialización a Shaders en GPU
    // -------------------------------------------------------------
    // Envía todos los structs de luces y arreglos a los uniforms del shader activo
    void applyToShader(const renderer::Shader& shader, const glm::vec3& viewPos) const;

private:
    DirectionalLight m_dirLight;
    std::vector<PointLight> m_pointLights;
    SpotLight m_spotLight;
    Material m_defaultMaterial;
    LightingModel m_currentModel{LightingModel::BLINN_PHONG};
    bool m_pointLightsEnabled{true};
};

// Alias para retrocompatibilidad
using LightingSystem = LightManager;

} // namespace lighting
