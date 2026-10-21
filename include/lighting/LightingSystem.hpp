#pragma once

#include "lighting/Light.hpp"
#include <glm/glm.hpp>

namespace renderer {
    class Shader;
}

namespace lighting {

/**
 * @brief Gestor del sistema de iluminación multi-fuente de la escena.
 * Controla el sol (direccional), farola (puntual), faros del kart (focal) y
 * el modelo de sombreado activo (Flat, Lambert, Blinn-Phong).
 */
class LightingSystem {
public:
    LightingSystem();
    ~LightingSystem() = default;

    // Configuración con valores iniciales equilibrados para la escena
    void init();

    // Actualiza la posición y orientación de los faros frontales del kart
    void updateKartHeadlights(const glm::vec3& kartPos, const glm::vec3& forwardDir);

    // Ajusta la posición de la farola en la pista
    void setPointLightPosition(const glm::vec3& position);

    // Envía todos los parámetros y estados de iluminación a los uniforms del shader activo
    void applyToShader(const renderer::Shader& shader, const glm::vec3& viewPos) const;

    // Conmutador del modelo de iluminación (Requisito UCV)
    void setLightingModel(LightingModel model);
    void cycleNextModel();
    LightingModel getLightingModel() const { return m_currentModel; }
    const char* getCurrentModelName() const { return getLightingModelName(m_currentModel); }

    // Control de activación de cada fuente de luz para pruebas y demostración
    void toggleDirectionalLight() { m_dirLight.enabled = !m_dirLight.enabled; }
    void togglePointLight()       { m_pointLight.enabled = !m_pointLight.enabled; }
    void toggleSpotLight()        { m_spotLight.enabled = !m_spotLight.enabled; }

    void setDirectionalLightEnabled(bool enabled) { m_dirLight.enabled = enabled; }
    void setPointLightEnabled(bool enabled)       { m_pointLight.enabled = enabled; }
    void setSpotLightEnabled(bool enabled)        { m_spotLight.enabled = enabled; }

    DirectionalLight& getDirectionalLight() { return m_dirLight; }
    PointLight& getPointLight()             { return m_pointLight; }
    SpotLight& getSpotLight()               { return m_spotLight; }

    const DirectionalLight& getDirectionalLight() const { return m_dirLight; }
    const PointLight& getPointLight() const             { return m_pointLight; }
    const SpotLight& getSpotLight() const               { return m_spotLight; }

    const Material& getDefaultMaterial() const { return m_defaultMaterial; }
    void setMaterialShininess(float shininess)  { m_defaultMaterial.shininess = shininess; }

private:
    DirectionalLight m_dirLight;
    PointLight m_pointLight;
    SpotLight m_spotLight;
    Material m_defaultMaterial;
    LightingModel m_currentModel{LightingModel::BLINN_PHONG};
};

} // namespace lighting
