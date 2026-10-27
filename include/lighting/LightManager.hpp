#pragma once

#include "lighting/Light.hpp"
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <array>

namespace renderer {
    class Shader;
}

namespace lighting {

/**
 * @brief Nombres precalculados de uniforms para una fuente puntual en GPU.
 * Evita la reconstrucción y alocaciones de heap de std::string en cada frame a 60 FPS.
 */
struct PointLightUniformNames {
    std::string enabled;
    std::string position;
    std::string ambient;
    std::string diffuse;
    std::string specular;
    std::string constant;
    std::string linear;
    std::string quadratic;
};

/**
 * @brief Gestor central de iluminacion para la escena.
 * Administra:
 * - 1 Luz Direccional (sol de la pista).
 * - Multiples Luces Puntuales dinamicas (farolas del circuito con atenuacion calculada).
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
    std::vector<PointLight>& getPointLights() { return m_pointLights; }
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
    // Conmutador del Modelo de Iluminacion
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

    // Nombres precalculados de uniforms para evitar alocaciones en applyToShader
    static const std::array<PointLightUniformNames, MAX_POINT_LIGHTS>& getPointLightUniformNames();

private:
    DirectionalLight m_dirLight;
    std::vector<PointLight> m_pointLights;
    SpotLight m_spotLight;
    Material m_defaultMaterial;
    LightingModel m_currentModel{LightingModel::BLINN_PHONG};
    bool m_pointLightsEnabled{true};
};

} // namespace lighting

